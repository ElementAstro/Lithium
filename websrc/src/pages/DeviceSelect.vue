<template>
  <q-page class="bg-grey-2">
    <div class="q-pa-md">
      <div class="text-h6 text-center">设备列表</div>

      <q-card class="q-mb-md" flat>
        <q-card-section class="row items-center q-gutter-sm">
          <q-select v-model="filterType" label="按类型筛选" :options="types" class="col-auto" />
          <q-select v-model="filterProtocol" label="按协议筛选" :options="protocols" class="col-auto" />
          <q-space />
          <q-btn icon="add" color="primary" @click="showAddDeviceDialog = true">
            添加设备
          </q-btn>
          <q-btn icon="cloud_upload" color="primary" @click="saveDevices">
            保存设备信息
          </q-btn>
        </q-card-section>
      </q-card>

      <q-card v-for="device in filteredDevices" :key="device.id" class="device-card" @click="editDevice(device)">
        <q-img :src="device.thumbnailUrl" aspect-ratio="1" class="device-thumbnail" transition="fade" />
        <q-card-section>
          <div class="text-h6">{{ device.name }}</div>
          <div class="text-subtitle2">{{ device.model }}</div>
          <q-list bordered class="q-mt-sm">
            <q-item>
              <q-item-label>类型:</q-item-label>
              <q-item-label class="text-weight-bold">{{ device.type }}</q-item-label>
            </q-item>
            <q-item>
              <q-item-label>协议:</q-item-label>
              <q-item-label class="text-weight-bold">{{ device.protocol }}</q-item-label>
            </q-item>
          </q-list>
        </q-card-section>
      </q-card>

      <q-dialog v-model="showAddDeviceDialog">
        <q-card>
          <q-card-section class="text-h5 text-center q-pb-1">
            {{ editingDevice ? '编辑设备' : '添加设备' }}
          </q-card-section>
          <q-card-section>
            <q-form @submit.prevent="editingDevice ? updateDevice : addDevice">
              <q-input v-model="newDevice.name" label="设备名称" required class="q-mb-sm" dense />
              <q-select v-model="newDevice.type" label="设备类型" :options="types" class="q-mb-sm" dense />
              <q-select v-model="newDevice.protocol" label="通信协议" :options="protocols" class="q-mb-sm" dense />
              <q-input v-model="newDevice.model" label="设备型号" class="q-mb-sm" dense />
              <q-input v-model="newDevice.thumbnailUrl" label="缩略图地址" class="q-mb-sm" dense />
              <div class="text-caption text-red q-mt-xs" v-if="!isNameUnique">
                同类型的同名设备已存在，请更换名称。
              </div>
              <div class="q-mt-md row justify-end">
                <q-btn type="submit" color="primary" :label="(editingDevice ? '保存' : '添加设备')" :disabled="!canSave" @click="showAddDeviceDialog = false"/>
                <q-btn label="取消" @click="showAddDeviceDialog = false" />
              </div>
            </q-form>
          </q-card-section>
        </q-card>
      </q-dialog>

    </div>
  </q-page>
</template>

<script>
import axios from 'axios'

const API_URL = '/api/devices'

export default {
  data() {
    return {
      devices: [],
      showAddDeviceDialog: false,
      editingDevice: null,
      newDevice: {
        name: '',
        model: '',
        thumbnailUrl: '',
        type: '',
        protocol: ''
      },
      types: [
        { value: 'camera', label: '相机' },
        { value: 'equatorial mount', label: '赤道仪' },
        { value: 'focuser', label: 'Focuser' },
        { value: 'filter wheel', label: '滤镜轮' }
      ],
      protocols: [
        { value: 'indi', label: 'INDI' },
        { value: 'ascom', label: 'ASCOM' },
        { value: 'native', label: '原生' }
      ],
      filterType: '',
      filterProtocol: ''
    }
  },

  computed: {
    canSave() {
      return (
        this.newDevice.name !== '' &&
        this.newDevice.type !== '' &&
        this.newDevice.protocol !== '' &&
        this.isNameUnique
      )
    },
    isNameUnique() {
      const { name, type } = this.newDevice
      return !this.devices.some(
        (device) => device.name === name && device.type === type && device !== this.editingDevice
      )
    },
    filteredDevices() {
      let filteredDevices = this.devices
      if (this.filterType !== '') {
        filteredDevices = filteredDevices.filter((device) => device.type === this.filterType)
      }
      if (this.filterProtocol !== '') {
        filteredDevices = filteredDevices.filter((device) => device.protocol === this.filterProtocol)
      }
      return filteredDevices
    }
  },

  mounted() {
    this.fetchDevices()
  },

  methods: {
    async fetchDevices() {
      try {
        const response = await axios.get(API_URL)
        this.devices = response.data
      } catch (error) {
        console.error(error)
      }
    },
    editDevice(device) {
      this.editingDevice = device
      this.showAddDeviceDialog = true
      this.newDevice = { ...device }
    },
    async addDevice() {
      try {
        const response = await axios.post(API_URL, this.newDevice)
        this.devices.push(response.data)
        this.newDevice = {
          name: '',
          model: '',
          thumbnailUrl: '',
          type: '',
          protocol: ''
        }
        this.showAddDeviceDialog = false
      } catch (error) {
        console.error(error)
      }

    },
    async updateDevice() {
      try {
        await axios.put(`${API_URL}/${this.editingDevice.id}`, this.newDevice)
        const index = this.devices.indexOf(this.editingDevice)
        this.devices.splice(index, 1, this.newDevice)
        this.editingDevice = null
        this.newDevice = {
          name: '',
          model: '',
          thumbnailUrl: '',
          type: '',
          protocol: ''
        }
        this.showAddDeviceDialog = false

      } catch (error) {
        console.error(error)
      }
    },
    async saveDevices() {
      try {
        const response = await axios.post(API_URL, this.devices)
        console.log(response.data)
      } catch (error) {
        console.error(error)
      }
    }
  }
}
</script>

<style scoped>
.device-card {
  box-shadow: 0px 2px 5px rgba(0, 0, 0, 0.15);
  transition: all 0.2s ease-in-out;
  cursor: pointer;
}

.device-card:hover {
  transform: scale(1.02);
}

.device-thumbnail {
  object-fit: cover;
  height: 0;
  padding-bottom: 100%;
}

.text-center {
  text-align: center;
}

.text-h6 {
  font-size: 1.5rem;
  font-weight: 500;
  margin-bottom: 16px;
}

.text-subtitle2 {
  font-size: 0.875rem;
  color: #999;
}

.text-weight-bold {
  font-weight: 500;
}

@media (max-width: 576px) {
  .q-card-section+.q-card-section {
    border-top: none;
    margin-top: -10px;
  }

  .q-gutter-sm>* {
    flex-grow: 1;
  }
}
</style> 