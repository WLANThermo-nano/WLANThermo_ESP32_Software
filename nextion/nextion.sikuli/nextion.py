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

# --- Device tab: select series (sys.argv[2] = "Enhanced" | "Basic") ---
# Check _sel first (orange = highly distinctive, low false-positive risk).
# Fall back to _gray with strict threshold 0.85 (all gray tabs share the same
# background color RGB(108,123,144); 0.70 caused wrong-tab false positives).
series = sys.argv[2]
series_gray = series + "_gray.png"
series_sel  = series + "_sel.png"
if exists(Pattern(series_sel).similar(0.80), 3):
    pass  # already selected
elif exists(Pattern(series_gray).similar(0.85), 5):
    click(Pattern(series_gray).similar(0.85))
else:
    raise Exception("Series tab not found: " + series)
sleep(1)

# --- Device tab: select model (sys.argv[3] = "NX3224K028" | "NX3224K024" | "NX3224T028") ---
# Same strategy: _sel (purple) first, then _gray at 0.85.
model = sys.argv[3]
model_gray = model + "_gray.png"
model_sel  = model + "_sel.png"
if exists(Pattern(model_sel).similar(0.80), 3):
    pass  # already selected
elif exists(Pattern(model_gray).similar(0.85), 5):
    click(Pattern(model_gray).similar(0.85))
else:
    raise Exception("Model not found: " + model)
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
