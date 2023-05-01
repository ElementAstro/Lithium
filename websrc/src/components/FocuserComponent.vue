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

    <q-card-section class="q-pa-sm row items-center justify-between">
      <div class="col-auto mr-4 flex items-center">
        <q-icon name="fas fa-map-marker-alt" class="q-mr-md" />
        <div class="text-h6">当前位置:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.currentPosition || '-' }}</div>
      </div>
      <div class="col-auto mr-4 flex items-center">
        <q-icon name="fas fa-thermometer-half" class="q-mr-md" />
        <div class="text-h6">当前温度:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.currentTemperature || '-' }} ℃</div>
      </div>
      <div class="col-auto mr-4 flex items-center">
        <q-icon name="fas fa-arrows-alt-h" class="q-mr-md" />
        <div class="text-h6">步长:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.stepSize || '-' }}</div>
      </div>
      <div class="col-auto mr-4 flex items-center">
        <q-icon name="fas fa-map-marker-alt" class="q-mr-md" />
        <div class="text-h6">最大位置:</div>
      </div>
      <div class="col-auto">
        <div class="text-body1">{{ data.maxPosition || '-' }}</div>
      </div>
    </q-card-section>

    <q-separator />

    <q-card-section class="q-pa-lg d-flex justify-content-center">
      <div class="row">
        <!-- 移动按钮组 -->
        <div class="col-lg-6 col-md-12 mb-lg-0 mb-md-4">
          <div class="btn-group btn-group-sm" role="group" aria-label="Basic example">
            <div class="row full-width justify-content-center">
              <div class="col-auto">
                <q-btn type="button" class="btn btn-secondary" label="-100" :disable="!data.connected" @click="move(-100)"
                  color="negative" />
              </div>
              <div class="col-auto">
                <q-btn type="button" class="btn btn-secondary" label="-10" :disable="!data.connected" @click="move(-10)"
                  color="warning" />
              </div>
              <div class="col-auto">
                <q-btn type="button" class="btn btn-success" label="-1" :disable="!data.connected" @click="move(-1)"
                  color="positive" />
              </div>
              <div class="col-auto">
                <q-btn type="button" class="btn btn-primary" label="+1" :disable="!data.connected" @click="move(1)"
                  color="primary" />
              </div>
              <div class="col-auto">
                <q-btn type="button" class="btn btn-secondary" label="+10" :disable="!data.connected" @click="move(10)"
                  color="warning" />
              </div>
              <div class="col-auto">
                <q-btn type="button" class="btn btn-secondary" label="+100" :disable="!data.connected" @click="move(100)"
                  color="negative" />
              </div>
            </div>
          </div>
        </div>
        <!-- 前往位置按钮组 -->
        <div class="col-lg-6 col-md-12">
          <div class="row">
            <div class="col-8">
              <q-input type="number" min="0" max="100000" class="form-control text-center text-bold" placeholder="前往..."
                v-model="targetPosition" />
            </div>
            <div class="col-4">
              <q-btn type="button" icon="fas fa-play" class="full-width q-ml-sm" label="前往" @click="gotoTargetPosition"
                :disable="!data.connected" />
            </div>
          </div>
        </div>
      </div>

    </q-card-section>

    <q-separator />

    <q-card-section class="q-pa-lg d-flex justify-content-between">
      <q-btn type="button" icon="fas fa-circle-arrow-up-right" class="q-mr-md" label="循环曝光" @click="startFocusing"
        :disable="!data.connected" flat color="positive" />
      <q-btn type="button" icon="fas fa-microchip-ai" class="q-ml-md" label="自动对焦"
        @click="$refs.offcanvasAutofocus.show()" :disable="!data.connected" flat color="negative" />
    </q-card-section>

  </q-card>

  <!-- 自动对焦抽屉 -->
  <q-drawer-layout container>
    <q-drawer ref="offcanvasAutofocus" show-if-above class="offcanvas-menu" side="right">
      <q-list inline>
        <q-item>
          <q-item-section>
            <q-toolbar flat>
              <q-btn round dense icon="fas fa-arrow-circle-left" @click="$refs.offcanvasAutofocus.hide()" />
              <q-toolbar-title class="text-h5 q-ml-md">自动对焦</q-toolbar-title>
            </q-toolbar>
          </q-item-section>
        </q-item>
        <q-item>
          <q-item-section>
            <div class="form-group q-mt-lg">
              <label class="text-h6">最小对焦值:</label>
              <q-input type="number" v-model="minFocusValue" class="q-mt-xs">
                <template v-slot:append>
                  {{ focusUnit }}
                </template>
              </q-input>
            </div>
          </q-item-section>
        </q-item>
        <q-item>
          <q-item-section>
            <div class="form-group q-mt-lg">
              <label class="text-h6">最大对焦值:</label>
              <q-input type="number" v-model="maxFocusValue" class="q-mt-xs">
                <template v-slot:append>
                  {{ focusUnit }}
                </template>
              </q-input>
            </div>
          </q-item-section>
        </q-item>
        <q-item>
          <q-item-section>
            <div class="form-group q-mt-lg">
              <label class="text-h6">步长:</label>
              <q-input type="number" v-model="focusStep" class="q-mt-xs">
                <template v-slot:append>
                  {{ focusUnit }}
                </template>
              </q-input>
            </div>
          </q-item-section>
        </q-item>
        <q-item>
          <q-item-section>
            <q-btn type="button" label="取消" @click="$refs.offcanvasAutofocus.hide()" color="negative" flat />
            <q-btn type="button" label="确定" @click="startAutofocus" :disable="!data.connected || minFocusValue === '' ||
              maxFocusValue === '' ||
              focusStep === ''" color="primary" flat />
          </q-item-section>
        </q-item>
      </q-list>
    </q-drawer>
  </q-drawer-layout>
</template>

<script>
export default {
  props: {
    data: {
      type: Object,
      required: true,
    },
  },
  data() {
    return {
      targetPosition: '',
      minFocusValue: '',
      maxFocusValue: '',
      focusStep: '',
      focusUnit: 'µm',
    };
  },
  methods: {
    // 移动
    move(delta) {
      if (this.data.connected) {
        this.$emit('move', delta);
      }
    },
    // 循环曝光
    startFocusing() {
      if (this.data.connected) {
        this.$emit('start-focusing');
      }
    },
    // 前往位置
    gotoTargetPosition() {
      const pos = Number(this.targetPosition);
      if (!isNaN(pos)) {
        this.$emit('move-to', pos);
      }
    },
    // 开始自动对焦
    startAutofocus() {
      console.log('start autofocus');
      console.log('min focus value:', this.minFocusValue);
      console.log('max focus value:', this.maxFocusValue);
      console.log('focus step:', this.focusStep);
    },
  },
};
</script>

<!-- 样式 -->
<style scoped>
.offcanvas-menu {
  width: 350px;
}

.offcanvas-title {
  font-size: 24px;
}
</style>

<style scoped>
  /* 重写 offcanvas-menu 样式 */
  .offcanvas-menu {
    max-width: 100vw !important;
    width: 100% !important;
    left: 0 !important;
    top: 0 !important;
  }
</style>
