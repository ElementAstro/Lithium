<template>
  <q-card class="q-my-md">
    <q-card-section class="">
      <ul class="list-group">
        <li class="list-group-item d-flex align-items-center justify-content-between">
          <div class="statusLed" id="focuser_status"></div>
          <span id="focuserConnText">电调连接</span>
        </li>
      </ul>
      <ul class="list-group text-small list-group-small">
        <li class="list-group-item d-flex mount-panel-item p-0">
          <div class="col p-1 m-0">
            <div class="text-center text-muted">温度</div>
            <div class="text-center shInField" id="focuser_temperature"></div>
          </div>
          <div class="col p-1 m-0">
            <div class="text-center text-muted ">位置</div>
            <div class="text-center shInField" id="focuser_position"></div>
          </div>
        </li>
      </ul>

      <div class="btn-group btn-group-sm w-100 mt-2" role="group" aria-label="Basic example">
        <q-btn type="button" class="btn btn-orange btn-sm px-2" :key="'focuser_movein_100'" @click="handleClick(-100)"
          data-param="-100" id="focuser_movein_100">
          -100
        </q-btn>
        <q-btn type="button" class="btn btn-light-blue btn-sm px-2" :key="'focuser_movein_10'" @click="handleClick(-10)"
          data-param="-10" id="focuser_movein_10">
          -10
        </q-btn>
        <q-btn type="button" class="btn btn-light-green btn-sm px-2 mr-1" :key="'focuser_movein_1'"
          @click="handleClick(-1)" data-param="-1" id="focuser_movein_1">
          -1
        </q-btn>
        <q-btn type="button" class="btn btn-light-green btn-sm px-2 ml-1" :key="'focuser_moveout_1'"
          @click="handleClick(1)" data-param="+1" id="focuser_moveout_1">
          +1
        </q-btn>
        <q-btn type="button" class="btn btn-light-blue btn-sm px-2" :key="'focuser_moveout_10'" @click="handleClick(10)"
          data-param="+10" id="focuser_moveout_10">
          +10
        </q-btn>
        <q-btn type="button" class="btn btn-orange btn-sm px-2" :key="'focuser_moveout_100'" @click="handleClick(100)"
          data-param="+100" id="focuser_moveout_100">
          +100
        </q-btn>
      </div>

      <div class="input-group my-3 input-group-sm">
        <div class="input-group-prepend">
          <q-btn type="button" class="btn btn-sm btn-outline-success" id="focuser_looping">
            <i class="fas fa-circle-arrow-up-right"></i>循环曝光
          </q-btn>
          <q-btn type="button" class="btn btn-sm btn-outline-info" id="goto_target_position">
            <i class="fas fa-play"></i>前往
          </q-btn>
        </div>
        <q-input type="number" min="0" max="100000000"
          class="form-control form-control-sm text-small text-center text-primary" placeholder=""
          id="focuser_target_position" aria-describedby="focuser-position" v-model="targetPosition" />
        <div class="input-group-append">
          <q-btn type="button" class="btn btn-sm btn-outline-danger" data-bs-toggle="offcanvas"
            data-bs-target="#offcanvas-autofocus" aria-controls="offcanvas-autofocus" id="autofocus">
            <i class="fas fa-microchip-ai"></i>自动对焦
          </q-btn>
        </div>
      </div>

      <div class="offcanvas offcanvas-end" tabindex="-1" id="offcanvas-autofocus"
        aria-labelledby="offcanvas-autofocusLabel">
        <div class="offcanvas-body" id="autofocus_offcanvas_body">
          <div class="row">
            <div class="col-md-12"></div>
          </div>
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
  },
  methods: {
    updateState() {
      // 处理新的数据对象，并更新组件状态
      this.$forceUpdate();
    },
  },
};
</script>
