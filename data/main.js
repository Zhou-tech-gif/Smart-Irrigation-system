// --- Mode Selection ---
const modeOutput = document.getElementById("mode_output");

document.getElementById("manual").addEventListener("click", async () => {
  const res = await fetch("/mode/manual");
  const text = await res.text();
  modeOutput.textContent = "selected manual mode";
});

document.getElementById("moisture").addEventListener("click", async () => {
  const res = await fetch("/mode/moisture");
  const text = await res.text();
  modeOutput.textContent = "Selected Mositure based mode";
});

document.getElementById("timer").addEventListener("click", async () => {
  const res = await fetch("/mode/timer");
  const text = await res.text();
  modeOutput.textContent = "Selected Timer Mode";
});

// --- Relay Control ---
const displayA_1 = document.getElementById("displayA_1");

document.getElementById("turnOn").addEventListener("click", async () => {
  const res = await fetch("/4/on");
  const text = await res.text();
  displayA_1.textContent = "Pump ON";
});

document.getElementById("turnOf").addEventListener("click", async () => {
  const res = await fetch("/4/off");
  const text = await res.text();
  displayA_1.textContent = "Pump of";
});

// --- Soil Moisture Auto Update ---
const moistureValue = document.getElementById("value");

async function updateMoisture() {
  try {
    const res = await fetch("/moisture");
    if (res.ok) {
      const data = await res.json(); // ESP sends {"moisture":42}
      moistureValue.textContent = data.moisture + " %";
    }
  } catch (err) {
    moistureValue.textContent = "Error ❌";
  }
}

setInterval(updateMoisture, 5000);
updateMoisture();

const saveBtn = document.getElementById('save');
const onInput = document.querySelector('input[name="on"]');
const offInput = document.querySelector('input[name="off"]');

saveBtn.addEventListener('click', async (e) => {
  e.preventDefault(); // prevent default form submission

  const formData = new URLSearchParams();
  formData.append('on', onInput.value);
  formData.append('off', offInput.value);

  try {
    const res = await fetch('/setThresholds', {
      method: 'POST',
      body: formData
    });
    const text = await res.text();
    moistureValue.textContent = text; // show response
  } catch (err) {
    moistureValue.textContent = "Error updating thresholds";
  }
});
// Schedule form
const scheduleForm = document.getElementById('scheduleForm');
const scheduleOutput = document.getElementById('schedule_output');

scheduleForm.addEventListener('submit', async (e) => {
  e.preventDefault(); // prevent page reload

  // Prepare form data
  const formData = new URLSearchParams();
  formData.append('hour', document.getElementById('hour').value);
  formData.append('minute', document.getElementById('minute').value);
  formData.append('duration', document.getElementById('duration').value);

  try {
    // Send POST request to ESP32
    const res = await fetch('/setSchedule', {
      method: 'POST',
      body: formData
    });
    const text = await res.text();

    // Display response
    scheduleOutput.textContent = text;
  } catch (err) {
    scheduleOutput.textContent = "Error updating schedule";
  }
});
