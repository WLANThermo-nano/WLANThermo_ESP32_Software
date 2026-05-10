import sys.argv
import java.lang.System as System
import org.sikuli.script.Screen as SikuliScreen

tempDir = System.getenv("RUNNER_TEMP")

# Dismiss update/version dialog if shown (may appear before editor is fully loaded)
if exists("Update.png", 10):
    click("Update.png")

# Wait for the editor to be fully loaded (Device button in toolbar)
wait("Device_button.png", 30)
sleep(1)

SikuliScreen().capture().save(tempDir, "debug_screen1.png")

# Open the Setting dialog via toolbar "Device" button
click("Device_button.png")
sleep(3)

SikuliScreen().capture().save(tempDir, "debug_screen2.png")

# --- Device tab: select series and model via anchor+offset ---
# Similarity matching is unreliable for tabs/buttons sharing the same background
# color (orange/purple/gray dominate the score, text differences are too small).
# Use the unique "Please Select The Model" blue heading as an anchor instead.
# All offsets measured from its center (245, 77) in CI screen coordinates.
#
# Series tab centers (y=154):  Basic=(362,154) Enhanced=(544,154)
# Model button centers (y=281): K024=(352,281) K028=(739,281) T028=(739,281)
series = sys.argv[2]
model  = sys.argv[3]

heading = find(Pattern("Please_Select_Model_heading.png").similar(0.80))

# Series tab (always click — idempotent, clicking selected tab keeps it selected)
if series == "Enhanced":
    click(heading.offset(299, 77))
elif series == "Basic":
    click(heading.offset(117, 77))
else:
    raise Exception("Unknown series: " + series)
sleep(1)

# Model button (always click — idempotent)
if model == "NX3224K028":
    click(heading.offset(494, 204))
elif model == "NX3224K024":
    click(heading.offset(107, 204))
elif model == "NX3224T028":
    click(heading.offset(494, 204))   # top-right in Basic grid, same position as K028
else:
    raise Exception("Unknown model: " + model)
sleep(1)

SikuliScreen().capture().save(tempDir, "debug_screen3.png")

# --- Navigate to Display tab (left panel) ---
click(Pattern("Display_tab.png").similar(0.80))
sleep(2)

SikuliScreen().capture().save(tempDir, "debug_screen4.png")

# --- Display tab: select direction ---
# sys.argv[1] = "0"   → click 90°  button (was "90 Horizontal" in old editor)
# sys.argv[1] = "180" → click 270° button (was "270 Horizontal" in old editor)
direction = sys.argv[1]
if direction not in ("0", "180"):
    raise Exception("Unknown direction argument: " + direction)

# Similarity matching fails for direction buttons: all four share the same
# background color, so the small number text (~10% of pixels) never dominates
# the score. Use the unique "Display direction" heading as an anchor and click
# at a fixed pixel offset — reliable because the dialog layout is fixed on CI.
anchor = find(Pattern("Display_direction_title.png").similar(0.80))
if direction == "0":
    click(anchor.offset(187, 70))    # 90°  Horizontal button
elif direction == "180":
    click(anchor.offset(559, 70))    # 270° Horizontal button
sleep(1)

SikuliScreen().capture().save(tempDir, "debug_screen5.png")

# --- Confirm settings ---
click("OK.png")
sleep(2)

# --- File → TFT file output → set output path → click Output ---
click("File.png")
click(Pattern("TFT_file_output.png").targetOffset(-85, 0))
wait("Output.png")
click(Pattern("Output.png").targetOffset(0, -50))
type(getParentPath())
click("Output.png")
wait(5)
