<template>
  <q-card class="q-pa-md">
    <q-card-section>
      <div class="row items-center justify-between">
        <div class="col-auto mr-4 flex items-center">
          <i class="fas fa-map-marker-alt fa-fw mr-1"></i>
          <div class="text-subtitle2">Alignment:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.alignment }}</div>
        </div>
      </div>
      <div class="row items-center mt-4 justify-between">
        <div class="col-auto mr-4 flex items-center">
          <i class="fas fa-ruler-horizontal fa-fw mr-1"></i>
          <div class="text-subtitle2">Polar Axis Position:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.polarAxisPosition || '-' }}°</div>
        </div>
        <div class="col-auto mr-4 flex items-center">
          <i class="fas fa-sliders-h fa-fw mr-1"></i>
          <div class="text-subtitle2">Focus Position:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.focusPosition || '-' }}°</div>
        </div>
        <div class="col-auto mr-4 flex items-center">
          <div class="text-subtitle2">Field of View:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.fieldOfView || '-' }}°</div>
        </div>
        <div class="ccol-auto mr-4 flex items-center">
          <div class="text-subtitle2">Observation Region:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.observationRegion || '-' }}</div>
        </div>

      </div>
      <div class="row items-center mt-4 justify-between">
        <div class="col-auto mr-4 flex items-center">
          <i class="fas fa-stopwatch fa-fw mr-1"></i>
          <div class="text-subtitle2">Exposure Time:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.exposureTime || '-' }} s</div>
        </div>
        <div class="col-auto mr-4 flex items-center">
          <i class="fas fa-microchip fa-fw mr-1"></i>
          <div class="text-subtitle2">Gain:</div>
        </div>
        <div class="col-auto">
          <div class="text-body1">{{ data.gain || '-' }} dB</div>
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
      </div>
    </q-card-section>

    <q-separator />

    <q-card-section>
      <div class="row q-gutter-md">
        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">极轴仪位置:</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="newPolarAxisPosition" suffix="度" />
            </div>
            <div class="col-auto">
              <q-btn label="应用" @click="applyPolarAxisPosition" />
            </div>
          </div>
        </div>
        <div class="col-auto pl-2">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">对焦位置:</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="newFocusPosition" suffix="度" />
            </div>
            <div class="col-auto">
              <q-btn label="应用" @click="applyFocusPosition" />
            </div>
          </div>
        </div>
      </div>

      <div class="row q-gutter-md">
        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">视场角:</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="newFieldOfView" suffix="度" />
            </div>
            <div class="col-auto">
              <q-btn label="应用" @click="applyFieldOfView" />
            </div>
          </div>
        </div>
        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">观测天区:</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="text" v-model="newObservationRegion" />
            </div>
            <div class="col-auto">
              <q-btn label="应用" @click="applyObservationRegion" />
            </div>
          </div>
        </div>
        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">赤经(RA):</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="ra" suffix="°" />
            </div>
          </div>
        </div>
        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">赤纬(DEC):</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="dec" suffix="°" />
            </div>
          </div>
        </div>
        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">海拔高度(ALT):</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="altitude" suffix="°" />
            </div>
          </div>
        </div>
        <div class="col-auto">
          <div class="row items-center q-mt-md">
            <div class="col-auto pr-2">
              <span class="text-weight-medium">方位角(AZ):</span>
            </div>
            <div class="col-auto pr-2">
              <q-input dense type="number" v-model="azimuth" suffix="°" />
            </div>
          </div>
        </div>
      </div>
        
      <div class="row q-gutter-md q-mt-lg">
        <div class="col-auto">
          <q-btn
            label="Save Settings"
            color="primary"
            class="q-mr-md"
            @click="saveSettings"
          />
        </div>
      </div>
    </q-card-section>
    <q-separator />
  </q-card>
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
      telescopes: [
        { value: 'hubble', label: 'Hubble' },
        { value: 'keck', label: 'Keck' },
        { value: 'james-webb', label: 'James Webb' },
      ],
      trackingModes: [
        { value: 'sidereal', label: 'Sidereal' },
        { value: 'solar', label: 'Solar' },
        { value: 'lunar', label: 'Lunar' },
        { value: 'custom', label: 'Custom' },
      ],
      telescope: this.data.telescope || '',
      ra: this.data.ra || '',
      dec: this.data.dec || '',
      altitude: this.data.altitude || '',
      azimuth: this.data.azimuth || '',
      slewTime: this.data.slewTime || '',
      trackingMode: this.data.trackingMode || '',
      guideRate: this.data.guideRate || '',
    };
  },
  methods: {
    updateTelescope() {
      this.$emit('update', { telescope: this.telescope });
    },
    updateRa() {
      this.$emit('update', { ra: this.ra });
    },
    updateDec() {
      this.$emit('update', { dec: this.dec });
    },
    updateAltitude() {
      this.$emit('update', { altitude: this.altitude });
    },
    updateAzimuth() {
      this.$emit('update', { azimuth: this.azimuth });
    },
    updateSlewTime() {
      this.$emit('update', { slewTime: this.slewTime });
    },
    updateTrackingMode() {
      this.$emit('update', { trackingMode: this.trackingMode });
    },
    updateGuideRate() {
      this.$emit('update', { guideRate: this.guideRate });
    },
    saveSettings() {
      const settings = {
        telescope: this.telescope,
        ra: this.ra,
        dec: this.dec,
        altitude: this.altitude,
        azimuth: this.azimuth,
        slewTime: this.slewTime,
        trackingMode: this.trackingMode,
        guideRate: this.guideRate,
      };
      this.$emit('save', settings);
    }
  },
};
</script>

<style>
/* 确保图标在按钮内居中 */
q-btn-toggle .q-btn-inner {
  display: flex;
  align-items: center;
  justify-content: center;
}
</style>