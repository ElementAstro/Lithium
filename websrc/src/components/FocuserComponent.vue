<template>
  <q-card class="q-my-md">
    <!-- 连接状态 -->
    <q-card-section class="d-flex flex-wrap align-items-center justify-content-between">
      <div class="d-flex align-items-center">
        <div class="statusLed" :class="{ 'led-on': data.connected }"></div>
        <span class="ml-2">{{ data.connected ? '已连接' : '未连接' }}</span>
      </div>
    </q-card-section>

    <q-card-section>
      <div class="row items-center justify-between">
        <div class="col-auto mr-4 flex items-center">
          <i class="fas fa-map-marker-alt fa-fw mr-1"></i>
          <div class="text-subtitle2">Current Position:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.currentPosition || '-' }}</div>
        </div>
        <div class="col-auto mr-4 flex items-center">
          <i class="fas fa-thermometer-half fa-fw mr-1"></i>
          <div class="text-subtitle2">Current Temperature:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.currentTemperature || '-' }} ℃</div>
        </div>
      </div>
      <div class="row items-center mt-4 justify-between">
        <div class="col-auto mr-4 flex items-center">
          <i class="fas fa-arrows-alt-h fa-fw mr-1"></i>
          <div class="text-subtitle2">Step Size:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.stepSize || '-' }}</div>
        </div>
        <div class="col-auto mr-4 flex items-center">
          <i class="fas fa-map-marker-alt fa-fw mr-1"></i>
          <div class="text-subtitle2">Max Position:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.maxPosition || '-' }}</div>
        </div>
      </div>
    </q-card-section>

    <q-separator />

    <q-card-section class="d-flex justify-content-center">
      <div class="row">
        <div class="col-6">
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
        <div class="col-6">
          <div class="btn-group btn-group-sm" role="group" aria-label="Basic example">
            <div class="row full-width justify-content-center">
              <div class="col-auto">
                <q-input type="number" min="0" max="100000" class="form-control text-center text-primary text-sm"
                  placeholder="前往..." v-model="targetPosition" />
              </div>
              <div class="col-auto">
                <q-btn type="button" icon="fas fa-play" class="btn btn-md btn-primary ml-2" label="前往"
                  @click="gotoTargetPosition" :disable="!data.connected" />
              </div>
              <div class="input-group input-group-sm w-100 d-flex align-items-center">



              </div>

            </div>
          </div>
        </div>
      </div>

    </q-card-section>

    <q-separator />
    <q-card-section>
      <div class="d-flex align-items-center">
        <!-- 循环曝光按钮 -->
        <div class="mr-3">
          <q-btn type="button" icon="fas fa-circle-arrow-up-right" class="btn btn-sm btn-outline-success" label="循环曝光"
            @click="startFocusing" :disable="!data.connected" />
        </div>
        <!-- 自动对焦按钮 -->
        <div class="ml-3">
          <q-btn type="button" icon="fas fa-microchip-ai" class="btn btn-sm btn-outline-danger" label="自动对焦"
            @click="$refs.offcanvasAutofocus.show()" :disable="!data.connected" />
        </div>
      </div>
    </q-card-section>
  </q-card>

  <q-expansion-item label="自动对焦">
    <q-card>
      <q-card-section>
        <div class="form-group q-mt-lg">
          <label class="text-body2">最小对焦值:</label>
          <q-input type="number" v-model="minFocusValue" class="q-mt-xs">
            <template v-slot:append>
              {{ focusUnit }}
            </template>
          </q-input>
        </div>
        <div class="form-group q-mt-lg">
          <label class="text-body2">最大对焦值:</label>
          <q-input type="number" v-model="maxFocusValue" class="q-mt-xs">
            <template v-slot:append>
              {{ focusUnit }}
            </template>
          </q-input>
        </div>
        <div class="form-group q-mt-lg">
          <label class="text-body2">步长:</label>
          <q-input type="number" v-model="focusStep" class="q-mt-xs">
            <template v-slot:append>
              {{ focusUnit }}
            </template>
          </q-input>
        </div>
      </q-card-section>
      <q-card-actions align="right">
        <q-btn flat label="取消" class="text-secondary" @click="$refs.offcanvasAutofocus.hide()" />
        <q-btn color="primary" label="确定" :disable="!data.connected" @click="startAutofocus"
          v-if="minFocusValue !== '' && maxFocusValue !== '' && focusStep !== ''" />
      </q-card-actions>
    </q-card>
  </q-expansion-item>
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

<style scoped>
.statusLed {
  width: 10px;
  height: 10px;
  border-radius: 50%;
  margin-right: 5px;
  transition: background-color 0.3s;
}

.led-on {
  background-color: #4caf50;
}

.shInField {
  font-size: 24px;
  font-weight: bold;
}

.offcanvas-title {
  font-size: 24px;
}
</style>