print("====Before_Build====")
Import("env")
import gzip
import shutil
import os
import subprocess
import binascii
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
import pip


# path configuration
web_ui_source_path = "./webui/old/"
web_ui_target_path = "./src/webui/"
web_ui_source_files = ["recoverymode.html", "restart.html"]

def install_package(package):
    subprocess.call(["pip", "install", "--upgrade", package])

install_package("beautifulsoup4")
install_package("html5lib")

from prepare_webui import WebUiPacker

def convert_web_ui_to_include_files():
    print("---- Convert Web UI to include files ----")

    # compress html files and create a uint8_t array
    for web_ui_file in web_ui_source_files:
        web_ui_file_header_array = web_ui_file + ".gz"

        # Set all entries in minify to "None" to avoid web access
        webPackerOptions = {
            "minify": {
                "JS": "online_andychilton", # "None" | "online_andychilton"
                "CSS": "online_andychilton", # "None" | "online_andychilton"
                "HTML": "online_andychilton" # "None" | "online_andychilton"
            }
        }
        webPacker = WebUiPacker(webPackerOptions)
        webPacker.log = True

        web_ui_file_inlined = webPacker.processFile(web_ui_source_path + web_ui_file)
        html_file_gzip = gzip.compress(web_ui_file_inlined.encode("utf-8"))
            
        html_file_gzip_hex = binascii.hexlify(html_file_gzip).decode("UTF-8").upper()
        html_file_gzip_hex_array = ["0x" + html_file_gzip_hex[i:i + 2] + ", " for i in range(0, len(html_file_gzip_hex), 2)]
        char_array_string = "const uint8_t " + web_ui_file_header_array.replace(".", "_") + "[] = {" + str("").join(html_file_gzip_hex_array) + "};"
        with open(web_ui_target_path + web_ui_file + ".gz.h", 'w') as f:
            f.write(char_array_string)

convert_web_ui_to_include_files()
