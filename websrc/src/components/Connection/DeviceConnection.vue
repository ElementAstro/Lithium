<template>
  <n-form
    ref="formRef"
    :model="form"
    :rules="rules"
    label-placement="left"
    :label-width="80"
  >
    <n-space justify="space-between" wrap>
      <!-- 服务器设置卡片 -->
      <n-card title="Server Settings" class="card">
        <n-grid :cols="24" :x-gap="12" >
          <!-- 名称输入 -->
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Name" path="name">
              <n-input v-model:value="form.name" placeholder="输入名称" />
            </n-form-item>
          </n-grid-item>
          <!-- 自动连接复选框 -->
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Auto Connect" path="autoConnect">
              <n-checkbox v-model="form.autoConnect" />
            </n-form-item>
          </n-grid-item>
          <!-- 模式选择 -->
          <n-grid-item :span="24" :md-span="12">
            <n-form-item label="Mode" path="mode">
              <n-radio-group v-model:value="form.mode" name="mode">
                <n-radio value="Local">Local</n-radio>
                <n-radio value="Remote">Remote</n-radio>
              </n-radio-group>
            </n-form-item>
          </n-grid-item>
          <!-- 主机地址和端口 -->
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Host" path="host">
              <n-input v-model:value="form.host" />
            </n-form-item>
          </n-grid-item>
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Port" path="port">
              <n-input-number v-model:value="form.port" />
            </n-form-item>
          </n-grid-item>
          <!-- Guiding选项 -->
          <n-grid-item :span="24" :md-span="12">
            <n-form-item label="Guiding" path="guiding">
              <n-select v-model:value="form.guiding" :options="guidingOptions" clearable />
            </n-form-item>
          </n-grid-item>
        </n-grid>
      </n-card>

      <!-- 设备设置卡片 -->
      <n-card title="Device Settings" class="card">
        <n-grid :cols="24" :x-gap="12" >
          <!-- 设备选择 -->
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Mount" path="mount">
              <n-select v-model:value="form.mount" placeholder="Select Mount" clearable />
            </n-form-item>
          </n-grid-item>
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Camera 1" path="camera1">
              <n-select v-model:value="form.camera1" :options="cameraOptions" />
            </n-form-item>
          </n-grid-item>
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Camera 2" path="camera2">
              <n-select v-model:value="form.camera2" placeholder="Select Camera 2" />
            </n-form-item>
          </n-grid-item>
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Focuser" path="focuser">
              <n-select v-model:value="form.focuser" :options="focuserOptions" />
            </n-form-item>
          </n-grid-item>
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Filter" path="filter">
              <n-select v-model:value="form.filter" :options="filterOptions" />
            </n-form-item>
          </n-grid-item>
        </n-grid>
      </n-card>

      <!-- 辅助设置卡片 -->
      <n-card title="Auxiliary Settings" class="card">
        <n-grid :cols="24" :x-gap="12" >
          <!-- Aux选项 -->
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Aux 1" path="aux1">
              <n-select v-model:value="form.aux1" placeholder="Aux 1" />
            </n-form-item>
          </n-grid-item>
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Aux 2" path="aux2">
              <n-select v-model:value="form.aux2" placeholder="Aux 2" />
            </n-form-item>
          </n-grid-item>
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Aux 3" path="aux3">
              <n-select v-model:value="form.aux3" placeholder="Aux 3" />
            </n-form-item>
          </n-grid-item>
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="Aux 4" path="aux4">
              <n-select v-model:value="form.aux4" placeholder="Aux 4" />
            </n-form-item>
          </n-grid-item>
          <!-- 新增选项 -->
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="New Option 1" path="newOption1">
              <n-input v-model:value="form.newOption1" placeholder="New Option 1" />
            </n-form-item>
          </n-grid-item>
          <n-grid-item :span="24" :md-span="6">
            <n-form-item label="New Option 2" path="newOption2">
              <n-input v-model:value="form.newOption2" placeholder="New Option 2" />
            </n-form-item>
          </n-grid-item>
        </n-grid>
      </n-card>
    </n-space>

    <!-- 保存和关闭按钮 -->
    <n-space justify="center" class="button-space">
      <n-button type="primary" @click="handleSave" :loading="loading" block>Save</n-button>
      <n-button @click="handleReset" block>Reset</n-button>
      <n-button type="error" @click="handleClose" block>Close</n-button>
    </n-space>
  </n-form>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import {
  NForm,
  NFormItem,
  NInput,
  NCheckbox,
  NRadioGroup,
  NRadio,
  NInputNumber,
  NSelect,
  NDivider,
  NButton,
  NCard,
  NGrid,
  NGridItem,
  NSpace,
  useMessage
} from 'naive-ui'

// 定义表单数据的接口类型
interface FormData {
  name: string
  autoConnect: boolean
  mode: 'Local' | 'Remote'
  host: string
  port: number
  guiding: string
  indiWebManager: boolean
  mount: string | null
  camera1: string
  camera2: string | null
  focuser: string
  filter: string
  aux1: string | null
  aux2: string | null
  aux3: string | null
  aux4: string | null
  newOption1: string
  newOption2: string
}

// 初始化表单数据
const formRef = ref(null)

const form = ref<FormData>({
  name: '',
  autoConnect: false,
  mode: 'Remote',
  host: '127.0.0.1',
  port: 7624,
  guiding: 'Internal',
  indiWebManager: false,
  mount: null,
  camera1: 'CCD Simulator',
  camera2: null,
  focuser: 'Focuser Simulator',
  filter: 'Filter Simulator',
  aux1: null,
  aux2: null,
  aux3: null,
  aux4: null,
  newOption1: '',
  newOption2: ''
})

// Guiding选项
const guidingOptions = [
  { label: 'Internal', value: 'Internal' },
  { label: 'External', value: 'External' }
]

// 模拟设备的选项
const cameraOptions = [{ label: 'CCD Simulator', value: 'CCD Simulator' }]

const focuserOptions = [{ label: 'Focuser Simulator', value: 'Focuser Simulator' }]

const filterOptions = [{ label: 'Filter Simulator', value: 'Filter Simulator' }]

// 表单验证规则
const rules = {
  name: [{ required: true, message: '请输入名称', trigger: 'blur' }],
  host: [{ required: true, message: '请输入主机地址', trigger: 'blur' }],
  port: [{ required: true, type: 'number' as const, message: '请输入端口号', trigger: 'blur' }]
}

// 加载状态
const loading = ref(false)

// 消息提示
const message = useMessage()

// 保存按钮处理函数
const handleSave = () => {
  loading.value = true
  formRef.value.validate((errors) => {
    if (!errors) {
      console.log('Form Data:', form.value)
      message.success('保存成功')
    } else {
      message.error('请检查表单')
    }
    loading.value = false
  })
}

// 重置按钮处理函数
const handleReset = () => {
  formRef.value.resetFields()
}

// 关闭按钮处理函数
const handleClose = () => {
  console.log('Form closed')
}
</script>

<style scoped></style>
