<template>
  <n-message-provider>
    <div id="app">
      <IndiForm />
    </div>
  </n-message-provider>
</template>

<script setup>
import IndiForm from './components/Connection/DeviceConnection.vue'
</script>

<style>
@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&display=swap');

#app {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
  margin: 0;
  background: url('https://example.com/space-background.jpg') no-repeat center center fixed;
  background-size: cover;
  font-family: 'Orbitron', sans-serif;
  color: #ffffff;
}

n-card {
  background: rgba(0, 0, 0, 0.8);
  border: 1px solid #00ffcc;
  box-shadow: 0 0 20px rgba(0, 255, 204, 0.5);
}

n-form-item {
  margin-bottom: 16px;
}

n-button {
  background: linear-gradient(45deg, #00ffcc, #0066ff);
  border: none;
  color: #ffffff;
  box-shadow: 0 0 10px rgba(0, 255, 204, 0.5);
}

n-button:hover {
  background: linear-gradient(45deg, #0066ff, #00ffcc);
}

n-input,
n-select,
n-checkbox,
n-radio-group {
  background: rgba(255, 255, 255, 0.1);
  border: 1px solid #00ffcc;
  color: #ffffff;
}

n-input::placeholder,
n-select::placeholder {
  color: #cccccc;
}
</style>
