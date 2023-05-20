<template>
  <q-card class="q-pa-md">
    <!-- 连接状态 -->
    <q-card-section class="q-pb-sm q-pl-md q-pr-md q-mb-md d-flex flex-wrap align-items-center justify-content-between">
      <div class="row d-flex align-items-center">
        <q-icon :name="data.connected ? 'fas fa-circle text-positive' : 'far fa-circle text-negative'" size="lg" />
        <span class="q-ml-md">{{ data.connected ? '已连接' : '未连接' }}</span>
        <q-select v-model="selectedDevice" :options="deviceOptions" label="选择设备" dense bordered class="q-ml-md">
        </q-select>
        <q-btn label="Scan" outline dense color="primary" class="q-ml-md" @click="scanDevices"></q-btn>
        <q-btn label="Connect" dense color="primary" class="q-ml-md" @click="connectDevice"></q-btn>
      </div>
    </q-card-section>

    <!-- 相机信息 -->
    <q-card-section v-if="data.type === 'Camera'" class="row items-center justify-between" v-cloak>
      <div v-for="(value, key) in data.info" :key="key" class="col-auto mr-4 flex items-center">
        <i :class="iconClass[key] + ' fa-fw mr-1'"></i>
        <div class="text-subtitle2">{{ displayName[key] }}:</div>
      </div>
      <div v-if="Object.keys(data.info).length === 0" class="col-auto text-body1 text-grey-8">暂无数据</div>
      <div v-else class="col-auto">
        <div v-for="(value, key) in data.info" :key="key" class="text-body1">{{ value }}</div>
      </div>
    </q-card-section>

    <!-- 其他类型设备信息 -->
    <q-card-section v-else class="row items-center justify-between">
      <div class="col-auto mr-4 flex items-center">
        <i class="fas fa-info-circle fa-fw mr-1"></i>
        <div class="text-subtitle2">Device Info:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.info }}</div>
      </div>
    </q-card-section>

    <q-separator />

    <!-- 设置 -->
    <q-card-section>
      <div class="row q-gutter-md">
        <div v-for="(value, key) in settings" :key="key" class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">{{ displayName[key] }}:</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="unsavedSettings[key]" />
            </div>
            <div class="col-auto">
              <q-btn label="Apply" @click="applySetting(key)" />
            </div>
          </div>
        </div>
      </div>
      <div class="row justify-end q-mt-md">
        <div class="col-auto">
          <q-btn label="Save Settings" dense color="primary" @click="saveSettings" />
        </div>
      </div>
    </q-card-section>
  </q-card>
</template>

<script>
export default {
  props: {
    data: {
      type: Object,
      required: true,
    },
    settings: {
      type: Object,
      required: true,
      default: () => ({
      }),
    },
  },
  data() {
    return {
      selectedDevice: null,
      unsavedSettings: {},
      deviceOptions: [],
      displayName: {
        ...this.settings
      },
      iconClass: {
        ...Object.fromEntries(Object.keys(this.settings).map(key => [key, 'fas fa-info-circle']))
      },
    };
  },
  methods: {
    scanDevices() {
      // 扫描设备
    },
    connectDevice() {
      // 连接设备
    },
    applySetting(key) {
      const value = parseInt(this.unsavedSettings[key]);
      if (!isNaN(value)) {
        this.unsavedSettings[key] = value;
      }
    },
    saveSettings() {
      const payload = {
        device: this.selectedDevice,
        type: this.data.type,
        info: {
          exposure: this.data.info.exposure,
          gain: this.data.info.gain,
          offset: this.data.info.offset,
          ...this.unsavedSettings
        },
      };
      // 发送保存设置的请求
    },
  },
}
</script>