<template>
    <q-card>
      <q-card-section class="row" v-for="(devices, index) in rowDevices" :key="devices.type">
        <div class="col">
          <h5>{{ devices.type }}</h5>
          <q-select v-model="selectedDevices[index]" :options="devices.devices" label-key="label" value-key="value"
            placeholder="请选择设备" />
        </div>
      </q-card-section>
  
      <q-card-actions align="right">
        <q-btn color="primary" label="确定" :disable="!isSubmitEnabled" @click="submit">
          <q-spinner v-if="isLoading" size="20px" color="white" />
        </q-btn>
        <q-btn color="negative" label="重置" @click="reset" />
      </q-card-actions>
    </q-card>
  </template>
  
  <script>
  import axios from "axios";
  
  export default {
    data() {
      return {
        deviceOptions: [
          {
            type: "相机",
            devices: [
              { label: "设备1-相机", value: "device1-相机", experimental: false },
              { label: "设备2-相机", value: "device2-相机", experimental: true },
              { label: "设备3-相机", value: "device3-相机", experimental: false },
              { label: "设备4-相机", value: "device4-相机", experimental: false }
            ]
          },
          {
            type: "赤道仪",
            devices: [
              { label: "设备1-赤道仪", value: "device1-赤道仪", experimental: false },
              { label: "设备2-赤道仪", value: "device2-赤道仪", experimental: false },
              { label: "设备3-赤道仪", value: "device3-赤道仪", experimental: true },
              { label: "设备4-赤道仪", value: "device4-赤道仪", experimental: false }
            ]
          },
          {
            type: "电调",
            devices: [
              { label: "设备1-电调", value: "device1-电调", experimental: false },
              { label: "设备2-电调", value: "device2-电调", experimental: false },
              { label: "设备3-电调", value: "device3-电调", experimental: false },
              { label: "设备4-电调", value: "device4-电调", experimental: false }
            ]
          },
          {
            type: "滤镜轮",
            devices: [
              { label: "设备1-滤镜轮", value: "device1-滤镜轮", experimental: false },
              { label: "设备2-滤镜轮", value: "device2-滤镜轮", experimental: false },
              { label: "设备3-滤镜轮", value: "device3-滤镜轮", experimental: true },
              { label: "设备4-滤镜轮", value: "device4-滤镜轮", experimental: false }
            ]
          },
          {
            type: "解析器",
            devices: [
              { label: "设备1-解析器", value: "device1-解析器", experimental: false },
              { label: "设备2-解析器", value: "device2-解析器", experimental: true },
              { label: "设备3-解析器", value: "device3-解析器", experimental: false },
              { label: "设备4-解析器", value: "device4-解析器", experimental: false }
            ]
          },
          {
            type: "导星软件",
            devices: [
              { label: "设备1-导星软件", value: "device1-导星软件", experimental: false },
              { label: "设备2-导星软件", value: "device2-导星软件", experimental: false },
              { label: "设备3-导星软件", value: "device3-导星软件", experimental: false },
              { label: "设备4-导星软件", value: "device4-导星软件", experimental: false }
            ]
          }
        ],
        selectedDevices: [[], [], [], [], [], []],
        isLoading: false
      };
    },
    computed: {
      rowDevices() {
        // 将设备按行分组
        return this.deviceOptions.reduce((rows, current) => {
          const rowIndex = rows.findIndex(row => row.type === current.type);
          if (rowIndex === -1) {
            rows.push({ type: current.type, devices: [current] });
          } else {
            rows[rowIndex].devices.push(current);
          }
          return rows;
        }, []);
      },
      isSubmitEnabled() {
        // 确定按钮是否启用
        return (
          this.selectedDevices.flat().length === 6 &&
          !this.isLoading &&
          !this.hasExperimentalDevices()
        );
      }
    },
    methods: {
      submit() {
        // 发送http请求
        this.isLoading = true;
        axios
          .post("/api/devices", this.selectedDevices)
          .then(response => {
            console.log("服务器返回结果：", response.data);
            // 显示成功消息
            this.$q.notify({
              message: "设备选择成功",
              color: "positive"
            });
          })
          .catch(error => {
            console.error("请求失败：", error);
            // 显示错误消息
            this.$q.dialog({
              title: "提示",
              message: "设备选择失败，请稍后重试",
              cancel: false,
              persistent: true,
              color: "negative"
            });
          })
          .finally(() => {
            this.isLoading = false;
          });
      },
      reset() {
        // 重置所有已选择的设备
        this.selectedDevices = [[], [], [], [], [], []];
      },
      hasExperimentalDevices() {
        // 是否选择了多个实验性设备
        const experimentalDevices = this.deviceOptions
          .filter(d => d.devices.some(device => device.experimental))
          .map(d => d.devices.filter(device => device.experimental).map(device => device.value))[0];
        const selectedExperimentalDevices = this.selectedDevices
          .flat()
          .filter(device => experimentalDevices.includes(device));
        return selectedExperimentalDevices.length > 1;
      }
    }
  };
  </script>
  