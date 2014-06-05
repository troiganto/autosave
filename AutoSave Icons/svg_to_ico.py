
import logging
import os
import shutil
from subprocess import check_output
from code import interact
from glob import glob
from time import sleep

INKSCAPE_PATH = r"C:\Program Files (x86)\Inkscape\inkscape.com"

NEEDS_LARGE_ICON = True
NEEDNT_LARGE_ICON = False

icons = [
    ("main", NEEDS_LARGE_ICON),
    ("disabled", NEEDS_LARGE_ICON),
    ("fileconnected", NEEDS_LARGE_ICON),
    ("cd5", NEEDNT_LARGE_ICON),
    ("cd4", NEEDNT_LARGE_ICON),
    ("cd3", NEEDNT_LARGE_ICON),
    ("cd2", NEEDNT_LARGE_ICON),
    ("cd1", NEEDNT_LARGE_ICON),
    ("cd0", NEEDNT_LARGE_ICON),
    ("ok", NEEDNT_LARGE_ICON),
    ]

# This one will be built from scratch.
specialIcon = "fileconnected"

class TempManager:

    def __init__(self):
        self.baseDir = os.getcwd()
        self.tempDir = os.path.join(self.baseDir, "temp")
        
    def __enter__(self):
        os.mkdir("temp")
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        os.chdir(self.baseDir)
        shutil.rmtree(self.tempDir)

    def push(self, pathToFile):
        fileSpec = os.path.split(pathToFile)[1]
        dest = os.path.join(self.tempDir, fileSpec)
        os.rename(pathToFile, dest)

    def keep_copy(self, pathToFile):
        fileSpec = os.path.split(pathToFile)[1]
        dest = os.path.join(self.tempDir, fileSpec)
        shutil.copy2(pathToFile, dest)

    def pop(self, fileSpec):
        old = os.path.join(self.tempDir, fileSpec)
        new = os.path.join(self.baseDir, fileSpec)
        os.rename(old, new)

    def clear():
        mask = os.path.join(self.tempDir, "*.*")
        files = glob(mask)
        for filePath in files:
            os.remove(filePath)
        

class IconBuilder(TempManager):

    @staticmethod
    def inkscapeCreate(inFile, size, outFile):
        args = [
            INKSCAPE_PATH,
            "--without-gui",
            "--export-background=black",
            "--export-background-opacity=0.0",
            "--export-width={:d}".format(size),
            "--export-png={}".format(outFile),
            inFile,
            ]
        check_output(args, shell=True)

    @staticmethod
    def imOverlay(inFile, overlay, overlaySize, outFile):
        args = [
            "convert",
            inFile,
            "null:",
            "(",
            overlay,
            "-resize", "{:d}x".format(overlaySize),
            ")",
            "-gravity", "southeast",
            "-layers", "composite",
            outFile,
            ]
        check_output(args, shell=True)

    @staticmethod
    def imReduceDepth(inFile, bitDepth, outFile):
        nColors = 2 << (bitDepth - 1)
        args = [
            "convert",
            inFile,
            "-background", "black",
            "-alpha", "off",
            "-transparent-color", "black",
            "-colors", "{:d}".format(nColors),
            "-depth", "{:d}".format(bitDepth),
            outFile,
            ]
        check_output(args, shell=True)

    def exportSizes(self, svgVersion, icon, needsLargeIcon=False):
        logging.info("exporting sizes for {} icon {}".format(
            svgVersion, icon))
        curDir = os.path.join("svg", svgVersion)
        logging.info("entering dir " + curDir)
        os.chdir(curDir)
        inFile = icon + ".svg"
        widths = [16, 32, 48]
        if needsLargeIcon:
            widths.append(256)
        for width in widths:
            outFile = "{} {} {}px.png".format(icon, svgVersion, width)
            logging.info("creating file " + outFile)
            self.inkscapeCreate(inFile, width, outFile)
            logging.info("Created, pushing to the temp dir")
            self.push(outFile)
        logging.info("All done! back to base dir")
        os.chdir(self.baseDir)

    def buildSizes(self, svgVersion, icon, needsLargeIcon=False):
        logging.info("building sizes for {} icon {}".format(
            svgVersion, icon))
        curDir = os.path.join("svg", svgVersion)
        logging.info("entering dir " + curDir)
        os.chdir(curDir)
        widths = [(16, 10), (32, 16), (48, 24)]
        if needsLargeIcon:
            widths.append((256, 128))
        logging.info("creating overlay icon")
        self.inkscapeCreate("main.svg", 256, "tmp.png")
        inFileDir = icon + " base"
        for mainWidth, overlayWidth in widths:
            inFile = os.path.join(
                inFileDir, "{:d}px.ico".format(mainWidth))
            outFile = "{} {} {:d}px.png".format(
                icon, svgVersion, mainWidth)
            logging.info("merging to create " + outFile)
            self.imOverlay(inFile, "tmp.png", overlayWidth, outFile)
            logging.info("Created, pushing to the temp dir")
            self.push(outFile)
        os.remove("tmp.png")
        logging.info("All done! back to base dir")
        os.chdir(self.baseDir)

    def reduceDepth(self, icon, bitDepth):
        logging.info("Going into temp dir for color reduction")
        logging.info("We want to reach {} bpp".format(bitDepth))
        os.chdir(self.tempDir)
        for width in (16, 32, 48):
            inFile = "{} {} {:d}px.png".format(icon, "flat", width)
            outFile = "{} {} {:d}px {:d}bpp.png".format(
                icon, "flat", width, bitDepth)
            logging.info("reducing colors for file " + outFile)
            self.imReduceDepth(inFile, bitDepth, outFile)
        logging.info("All done! back to base dir")
        os.chdir(self.baseDir)

    def packToIcon(self, icon):
        logging.info("Going into temp dir for packing")
        os.chdir(self.tempDir)
        args = ["convert"]
        # Add color-reduced images
        for bitDepth in (4, 8):
            for width in (48, 32, 16):
                inFile = "{} {} {:d}px {:d}bpp.png".format(icon, "flat",
                                                       width, bitDepth)
                logging.info("adding file " + inFile)
                args.append(inFile)
        # Add 32-bit pics
        for width in (256, 48, 32, 16):
            inFile = "{} {} {:d}px.png".format(icon, "true", width, bitDepth)
            if os.path.isfile(inFile):
                logging.info("adding file " + inFile)
                args.append(inFile)
        # Execute
        logging.info("packing the icon now")
        args.append(icon + ".ico")
        check_output(args, shell=True)
        logging.info("All done! Back to base dir")
        os.chdir(self.baseDir)




# MAIN

if __name__ == "__main__":
    logging.basicConfig(level="INFO")
    with IconBuilder() as ib:
        for icon, needsLargeIcon in icons:
            # Get PNGs in different sizes
            if icon == specialIcon:
                ib.buildSizes("true", icon, needsLargeIcon)
                ib.buildSizes("flat", icon)
            else:
                ib.exportSizes("true", icon, needsLargeIcon)
                ib.exportSizes("flat", icon)
            # Get color-reduces versions
            ib.reduceDepth(icon, 8)
            ib.reduceDepth(icon, 4)
            # Pack into icon
            ib.packToIcon(icon)
            ib.pop(icon + ".ico")
