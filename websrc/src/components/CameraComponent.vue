<template>
  <q-card class="q-pa-md">
    <!-- 连接状态 -->
    <q-card-section class="q-pb-sm q-pl-md q-pr-md q-mb-md d-flex flex-wrap align-items-center justify-content-between">
      <div class="row d-flex align-items-center">
        <q-icon :name="data.connected ? 'fas fa-circle' : 'far fa-circle'" size="lg" color="positive" />
        <span class="q-ml-md">{{ data.connected ? '已连接' : '未连接' }}</span>
        <q-select v-model="selectedDevice" :options="deviceOptions" label="选择设备" dense bordered class="q-ml-md">
        </q-select>
        <q-btn label="Scan" outline dense color="primary" class="q-ml-md" @click="scanDevices"></q-btn>
        <q-btn label="Connect" dense color="primary" class="q-ml-md" @click="connectDevice"></q-btn>
      </div>
    </q-card-section>
    <q-separator />
    <q-card-section class="q-pa-sm row items-center justify-between">
      <div class="col-auto mr-4 flex items-center">
        <i class="fas fa-camera-retro fa-fw mr-1"></i>
        <div class="text-subtitle2">Camera:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.camera }}</div>
      </div>
    </q-card-section>
    <q-separator />
    <q-card-section class="row items-center justify-between">
      <div class="col-auto mr-4 flex items-center">
        <i class="fas fa-ruler-horizontal fa-fw mr-1"></i>
        <div class="text-subtitle2">Pixel Binning:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.pixelBinning || '-' }}</div>
      </div>
      <div class="col-auto mr-4 flex items-center">
        <i class="fas fa-sliders-h fa-fw mr-1"></i>
        <div class="text-subtitle2">Bias Level:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.biasLevel || '-' }}</div>
      </div>
      <div class="col-auto mr-4 flex items-center">
        <div class="text-subtitle2">Resolution:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.resolution || '-' }}</div>
      </div>
      <div class="col-auto mr-4 flex items-center">
        <div class="text-subtitle2">Sensor Type:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.sensorType || '-' }}</div>
      </div>
    </q-card-section>
    <q-card-section class="row items-center justify-between mt-4">
      <div class="col-auto mr-4 flex items-center">
        <i class="fas fa-stopwatch fa-fw mr-1"></i>
        <div class="text-subtitle2">Exposure Time:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.exposureTime || '-' }} ms</div>
      </div>
      <div class="col-auto mr-4 flex items-center">
        <i class="fas fa-microchip fa-fw mr-1"></i>
        <div class="text-subtitle2">Gain:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.gain || '-' }}</div>
      </div>
      <div class="col-auto mr-4 flex items-center">
        <i class="fas fa-thermometer-half fa-fw mr-1"></i>
        <div class="text-subtitle2">Temperature:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.temperature || '-' }} ℃</div>
      </div>
      <div class="col-auto mr-4 flex items-center">
        <i class="fas fa-tint fa-fw mr-1"></i>
        <div class="text-subtitle2">Humidity:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.humidity || '-' }} %</div>
      </div>
    </q-card-section>
    <q-separator />

    <q-card-section>
      <div class="row q-gutter-md">
        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">Exposure Time:</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="newExposureTime" suffix="ms" />
            </div>
            <div class="col-auto">
              <q-btn label="Apply" @click="applyExposureTime" />
            </div>
          </div>
        </div>
        <div class="col-auto pl-2">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">Gain:</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="newGain" suffix="dB" />
            </div>
            <div class="col-auto">
              <q-btn label="Apply" @click="applyGain" />
            </div>
          </div>
        </div>
      </div>



      <div class="row q-gutter-md">
        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">Pixel Binning:</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="newPixelBinning" />
            </div>
            <div class="col-auto">
              <q-btn label="Apply" @click="applyPixelBinning" />
            </div>
          </div>

        </div>

        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">Bias Level:</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="newBiasLevel" suffix="DN" />
            </div>
            <div class="col-auto">
              <q-btn label="Apply" @click="applyBiasLevel" />
            </div>
          </div>

        </div>

      </div>
    </q-card-section>

    <q-separator />
    <q-card-section>
      <q-card class="mb-6">
        <div class="card-header row items-center justify-between">
          <div class="col-auto flex items-center">
            <i class="fas fa-thermometer-half fa-fw mr-1"></i>
            <div class="ml-2 text-h5">Temperature Monitoring</div>
          </div>
          <div class="col-auto">
            <q-icon name="help" class="cursor-pointer" />
          </div>
        </div>
        <q-line v-if="temperatureData.datasets[0].data.length > 1" :data="temperatureData" />
      </q-card>


    </q-card-section>
  </q-card>
</template>

<script>
import { Line } from "vue-chartjs";

export default {
  components: {
    QLine: Line,
  },
  props: {
    data: {
      type: Object,
      required: true,
    },
    settings: {
      type: Array,
      required: true,
      default: () => ["exposureTime", "gain", "pixelBinning", "biasLevel"],
    },
  },
  data() {
    return {
      selectedSetting: 'exposureTime',
      newExposureTime: null,
      newGain: null,
      newPixelBinning: null,
      newBiasLevel: null,
      unsavedSettings: {},
      settingOptions: ['exposureTime', 'gain', 'pixelBinning', 'biasLevel'].map((setting) => ({ label: setting, value: setting })),
      temperatureData: {
        datasets: [
          {
            label: 'Temperature (℃)',
            borderColor: '#42A5F5',
            pointRadius: 0,
            data: [],
          },
        ],
      },
    };
  },
  created() {
    this.selectedSetting = this.settings[0];
  },
  methods: {
    updateState() {
      // 处理新的数据对象，并更新组件状态
      this.$forceUpdate();
      this.updateTemperatureChart();
    },
    settingChanged() {
      this.newExposureTime = null;
      this.newGain = null;
      this.newPixelBinning = null;
      this.newBiasLevel = null;
      switch (this.selectedSetting) {
        case "exposureTime":
          this.unsavedSettings.exposureTime = this.data.exposureTime;
          break;
        case "gain":
          this.unsavedSettings.gain = this.data.gain;
          break;
        case "pixelBinning":
          this.unsavedSettings.pixelBinning = this.data.pixelBinning;
          break;
        case "biasLevel":
          this.unsavedSettings.biasLevel = this.data.biasLevel;
          break;
      }
    },
    applyExposureTime() {
      const value = parseInt(this.newExposureTime);
      if (!isNaN(value) && value >= 0) {
        this.unsavedSettings.exposureTime = value;
      }
    },
    applyGain() {
      const value = parseInt(this.newGain);
      if (!isNaN(value) && value >= 0) {
        this.unsavedSettings.gain = value;
      }
    },
    applyPixelBinning() {
      const value = parseInt(this.newPixelBinning);
      if (!isNaN(value) && value >= 1) {
        this.unsavedSettings.pixelBinning = value;
      }
    },
    applyBiasLevel() {
      const value = parseInt(this.newBiasLevel);
      if (!isNaN(value) && value >= 0) {
        this.unsavedSettings.biasLevel = value;
      }
    },
    saveSettings() {
      const payload = {};
    },
    updateTemperatureChart() {
      const now = new Date();
      const timeString = now.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
      const temperature = this.data.temperature;
      this.temperatureData.datasets[0].data.push({ x: timeString, y: temperature });
      if (this.temperatureData.datasets[0].data.length > 10) {
        this.temperatureData.datasets[0].data.shift();
      }
    },

  },
  mounted() {
    this.$nextTick(() => {
      this.updateTemperatureChart();
      this.renderChart(this.temperatureData, {
        responsive: true,
        maintainAspectRatio: false,
        legend: {
          display: false,
        },
        scales: {
          xAxes: [{
            type: 'time',
            time: {
              unit: 'minute',
              displayFormats: {
                minute: 'h:mmA',
              },
            },
          }],
          yAxes: [{
            ticks: {
              min: 0,
              max: 40,
            },
          }],
        },
      });
    });
  },
};
</script>

