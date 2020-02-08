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
web_ui_path = "./webui/"
target_path = "./src/webui/"
web_ui_source_files = ["index.html", "fwupdate.html", "recoverymode.html"]

def install_package(package):
    subprocess.call(["pip","install","--upgrade",package])
install_package("html_utils_becothal")
from html_utils import HTML


def convert_web_ui_to_include_files():
    print("---- Convert Web UI to include files ----")

    # add logo
    if str(env["PIOENV"]).startswith("nano"):
        shutil.copyfile(web_ui_path + "WLANThermoNano.png", web_ui_path + "WLANThermoLogo.png")
    else:
        shutil.copyfile(web_ui_path + "WLANThermoMini.png", web_ui_path + "WLANThermoLogo.png")

    # compress html files and create a uint8_t array
    for web_ui_file in web_ui_source_files:
        web_ui_file_inlined = web_ui_file + "_inlined.html"
        web_ui_file_header_array = web_ui_file + ".gz"
        html_file = HTML()
        html_file.read_file(web_ui_path + web_ui_file)
        html_file.inline_css()
        html_file.inline_js()
        html_file.remove_comments("", "<!--", "-->")
        html_file.images_to_base64()
        html_file.write_file(web_ui_file_inlined)
        with open(web_ui_file_inlined, "rb") as f:
            html_file_gzip = gzip.compress(f.read())
        html_file_gzip_hex = binascii.hexlify(html_file_gzip).decode("UTF-8").upper()
        html_file_gzip_hex_array = ["0x" + html_file_gzip_hex[i:i + 2] + ", " for i in range(0, len(html_file_gzip_hex), 2)]
        char_array_string = "const uint8_t " + web_ui_file_header_array.replace(".", "_") + "[] = {" + str("").join(html_file_gzip_hex_array) + "};"
        with open(target_path + web_ui_file + ".gz.h", 'w') as f:
            f.write(char_array_string)
        os.remove(web_ui_file_inlined)

    #remove logo
    os.remove(web_ui_path + "WLANThermoLogo.png")

convert_web_ui_to_include_files()