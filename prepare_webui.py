#! /usr/bin/env python
# NOTE: USE PYTHON 3

from bs4 import BeautifulSoup, Comment, NavigableString
# from enum import Enum  # for enum34, or the stdlib version ... TODO

import re, os, base64
import requests # for https://javascript-minifier.com/raw | https://cssminifier.com/

#JsMinifier = Enum('None', 'online_andychilton')

class WebUiPacker:
    def __init__(self, options):
        self.options = WebUiPacker.getDefaultOptions(options)
        self.log = False

    @staticmethod
    def getDefaultOptions(options):
        defaultOptions = {
            "inline": {
                "JS": True,
                "CSS": True,
                "IMG": True
            },
            "remove": {
                "level_empty_lines": 1,
                "comments": True
            },
            "minify": {
                "JS": "online_andychilton", # "None" | "online_andychilton"
                "CSS": "online_andychilton", # "None" | "online_andychilton"
                "HTML": "online_andychilton" # "None" | "online_andychilton"
            },
            "prettify": {
                "HTML": False
            }
        }
        if not options:
            return defaultOptions
        for key in defaultOptions:
            if key in options:
                defaultOptions[key].update(options[key])
        return defaultOptions

    def __getOption(self, *keys):
        opts = self.options
        for key in keys:
            if not key in opts:
                raise NameError('Unknown config key', key, keys)
            opts = opts[key]
        return opts

    def __log(self, *msgs):
        if self.log:
            out = "[WebUiPacker]"
            print(out, msgs);

    @staticmethod
    def __isSoupEmptyLine(item):
        if isinstance(item, NavigableString):
            if len(item) == 1 and item == "\n":
                return True
        return False

    @staticmethod
    def minifyHtml(content, opt):
        ## ALTERNATIVE using nodeJS
        ## https://github.com/mishoo/UglifyJS2 - with nodeJs
        if opt == "online_andychilton":
            url = 'https://html-minifier.com/raw'
            #print("SEND CONTENT: ", content)
            response = requests.post(url, data={'input': content})
            minified = response.text
            return minified
        return content

    @staticmethod
    def minifyCSS(content, opt):
        ## ALTERNATIVE using nodeJS
        ## https://github.com/mishoo/UglifyJS2 - with nodeJs
        if opt == "online_andychilton":
            url = 'https://cssminifier.com/raw'
            #print("SEND CONTENT: ", content)
            response = requests.post(url, data={'input': content})
            minified = response.text
            return minified
        return content

    @staticmethod
    def minifyJs(content, opt):
        ## ALTERNATIVE using nodeJS
        ## https://github.com/mishoo/UglifyJS2 - with nodeJs
        if opt == "online_andychilton":
            url = 'https://javascript-minifier.com/raw'
            #print("SEND CONTENT: ", content)
            response = requests.post(url, data={'input': content})
            minified = response.text
            return minified
        return content

    @staticmethod
    def inlineCSS(linkItem):
        src = linkItem["href"]
        try:
            with open(src, 'r', encoding="utf8") as handle:
                linkItem.name = "style"
                linkItem.clear()
                linkItem.attrs.clear()# = {}
                content = handle.read()
                linkItem.string = "\n" + content + "\n"
        except:
            print("Couln't inline CSS: ", src)
            raise

    @staticmethod
    def inlineIMG(imgItem):
        src = imgItem["src"]
        try:
            imgItem["src"] = WebUiPacker.imageFileToBase64(src)
        except:
            print("Couln't inline IMG: ", src)
            raise

    @staticmethod
    def inlineJS(scriptItem):
        src = scriptItem["src"]
        try:
            with open(src, 'r', encoding="utf8") as handle:
                content = handle.read()
                scriptItem.string = "\n" + content + "\n"
                del scriptItem["src"]
                scriptItem["type"] = "text/javascript"
        except:
            print("Couln't inline JS: ", src)
            raise

    @staticmethod
    def imageFileToBase64(imageFilePath):
        # print("imageFilePath:", imageFilePath)
        with open(imageFilePath, 'rb') as handle:
            data = handle.read()
        base64Encoded = base64.b64encode(data)
        ext = imageFilePath.split('.')[-1]
        prefix = f'data:image/{ext};base64,'
        return prefix+base64Encoded.decode('utf-8')
        return "Foo"

    def __unsetHtmlPrettifyOnDemand(self, reason):
        if not self.__getOption("prettify", "HTML"):
            return
        self.options["prettify"]["HTML"] = False
        self.__log("The ['prettify']['HTML'] option is set to False: ", reason)

    def processContent(self, content, workingDir):
        ## options = __getDefaultOptions(options)
        soup = BeautifulSoup(content, 'html5lib')
        #soup = BeautifulSoup(content, 'html.parser')
        curDir = os.getcwd()

        # remove all comments
        if  self.__getOption("remove", "comments"):
            self.__log("Remove comments")
            for comments in soup.findAll(text=lambda text:isinstance(text, Comment)):
                comments.extract()

        # skip empty lines on root element (TODO: depth)
        if self.__getOption("remove","level_empty_lines") > 0:
            self.__log("Remove empty lines (root level)")
            for child in soup:
                if WebUiPacker.__isSoupEmptyLine(child):
                    child.extract()
                    #child.decompose()

        if self.__getOption("inline","CSS"):
            self.__log("Inline CSS")
            os.chdir(workingDir)
            for style in soup.findAll('link', href=re.compile(r".+"), rel="stylesheet", type="text/css"):
                WebUiPacker.inlineCSS(style)
            os.chdir(curDir)

        if self.__getOption("inline","JS"):
            self.__log("Inline JS")
            os.chdir(workingDir)
            for jsScript in soup.findAll('script', src=re.compile(r".+")):
                WebUiPacker.inlineJS(jsScript)
            os.chdir(curDir)

        if self.__getOption("inline","IMG"):
            self.__log("Inline IMG")
            os.chdir(workingDir)
            for img in soup.findAll('img', src=re.compile(r".+")):
                if not img["src"].startswith("data:"):
                    WebUiPacker.inlineIMG(img)
            os.chdir(curDir)

        cssMinify = self.__getOption("minify","CSS")
        jsMinify = self.__getOption("minify","JS")
        htmlMinify = self.__getOption("minify","HTML")
        
  

        if htmlMinify == "online_andychilton":
            if cssMinify == htmlMinify:
                self.__log("Skip CSS Minify as it will be a part of the HTML minify step.")
                cssMinify = "None"
            if jsMinify == htmlMinify:
                self.__log("Skip JS Minify as it will be a part of the HTML minify step.")
                jsMinify = "None"

        if cssMinify != "None":
            self.__log("Minify CSS")
            self.__unsetHtmlPrettifyOnDemand("Recurs with the minify option for CSS.")
            for style in soup.findAll('style'):
                text = style.string
                minified = WebUiPacker.minifyCSS(text, cssMinify)
                style.string = "\n" + minified + "\n"

        if jsMinify != "None":
            self.__log("Minify JS")
            self.__unsetHtmlPrettifyOnDemand("Recurs with the minify option for JS.")
            for jsScript in soup.findAll('script', type="text/javascript"):
                if "src" in jsScript.attrs:
                    continue
                text = jsScript.string
                minified = WebUiPacker.minifyJs(text, jsMinify)
                jsScript.string = minified

        if htmlMinify != "None":
            self.__unsetHtmlPrettifyOnDemand("Recurs with the minify option for HTML.")
            self.__log("Minify HTML")
            text = str(soup)
            minified = WebUiPacker.minifyHtml(text, htmlMinify)
            content = minified
        else:
            if self.__getOption("prettify", "HTML"):
                content = soup.prettify()
            else:
                content = str(soup)

        #htmlStr = htmlObj.prettify()
        #htmlStr = str(htmlObj)
        return content #.encode("utf-8").decode("utf-8")

    def processFile(self, filePath):
        with open(filePath, 'r', encoding="utf8") as handle:
            content = handle.read()

        htmlStr = self.processContent(content, os.path.dirname(filePath))
        return htmlStr

### USAGE EXAMPLE
### options = WebUiPacker.getDefaultOptions(None)
### options["inline"]["IMG"] = False
### webUiPacker = WebUiPacker(options)
### webUiPacker.log = True
### 
### out = webUiPacker.processFile("webuiPacker/index.html")
### 
### with open("webuiPacker/_test_out.html", "w", encoding="utf8") as handle:
###     handle.write(out)
### 
### #print(out)