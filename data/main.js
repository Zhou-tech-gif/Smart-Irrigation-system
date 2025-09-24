 async function getMoisture() {
      try {
        let response = await fetch('/moisture');
        let data = await response.json();
        document.getElementById('value').textContent = data.moisture;
      } catch (e) {
        console.error("Error fetching data:", e);
      }
    }
    // Update every 2 seconds
    setInterval(getMoisture, 2000);
    getMoisture();