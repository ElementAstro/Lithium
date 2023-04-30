<template>
  <div>
    <q-splitter v-model="splitterModel" style="height: 100%">
      <template v-slot:before>
        <q-tabs v-model="tab" class="text-teal bg-white shadow-2" vertical>
          <q-tab name="camera" icon="fas fa-camera" label="Camera"></q-tab>
          <q-tab name="mount" icon="fas fa-mountain" label="Mount"></q-tab>
          <q-tab name="focuser" icon="fas fa-adjust" label="Focuser"></q-tab>
          <q-tab name="filterWheel" icon="fas fa-filter" label="Wheel"></q-tab>
          <q-tab name="solver" icon="fas fa-search" label="Solver"></q-tab>
        </q-tabs>
      </template>

      <template v-slot:after>
        <q-tab-panels
          v-model="tab"
          animated
          swipeable
          transition-prev="jump-up"
          transition-next="jump-up"
        >
          <q-tab-panel name="camera">
            <camera-component :data="cameraData" @send-message="sendMessage" ref="cameraComponent" />
          </q-tab-panel>

          <q-tab-panel name="mount">
            <mount-component :data="mountData" @send-message="sendMessage" ref="mountComponent" />
          </q-tab-panel>

          <q-tab-panel name="focuser">
            <focuser-component :data="focuserData" @send-message="sendMessage" ref="focuserComponent" />
          </q-tab-panel>

          <q-tab-panel name="filterWheel">
            <filter-wheel-component :data="filterWheelData" @send-message="sendMessage" ref="filterWheelComponent" />
          </q-tab-panel>

          <q-tab-panel name="solver">
            <solver-component :data="solverData" @send-message="sendMessage" ref="solverComponent" />
          </q-tab-panel>
        </q-tab-panels>
      </template>
    </q-splitter>
  </div>
</template>

<script>
import { ref } from "vue";
import CameraComponent from "../components/CameraComponent.vue";
import MountComponent from "../components/MountComponent.vue";
import FocuserComponent from "../components/FocuserComponent.vue";
import FilterWheelComponent from "../components/FilterWheelComponent.vue";
import SolverComponent from "../components/SolverComponent.vue";

export default {
  components: {
    "camera-component": CameraComponent,
    "mount-component": MountComponent,
    "focuser-component": FocuserComponent,
    "filter-wheel-component": FilterWheelComponent,
    "solver-component" : SolverComponent
  },
  setup() {
    const cameraData = ref({
      //...
    });
    const mountData = ref({
      //...
    });
    const focuserData = ref({
      //...
    });
    const filterWheelData = ref({
      //...
    });
    const solverData = ref({
      //...
    });

    const tab = ref("camera");
    const splitterModel = ref(10);
    const wsClient = new WebSocket("ws://localhost:8080");

    // 发送消息到WebSocket服务器
    const sendMessage = (message) => {
      wsClient.send(JSON.stringify(message));
    };

    // 处理来自服务器的消息，根据消息类型更新对应的组件状态
    wsClient.onmessage = (event) => {
      const data = JSON.parse(event.data);
      if (data.type === "camera") {
        cameraData.value = data.payload;
        this.$refs.cameraComponent.updateState();
      } else if (data.type === "mount") {
        mountData.value = data.payload;
        this.$refs.mountComponent.updateState();
      } else if (data.type === "focuser") {
        focuserData.value = data.payload;
        this.$refs.focuserComponent.updateState();
      } else if (data.type === "filterWheel") {
        filterWheelData.value = data.payload;
        this.$refs.filterWheelComponent.updateState();
      } else if (data.type === "solver") {
        solverData.value = data.payload;
        this.$refs.solverComponent.updateState();
      }
    };

    return {
      cameraData,
      mountData,
      focuserData,
      filterWheelData,
      solverData,
      tab,
      splitterModel,
      wsClient,
      sendMessage,
    };
  },
};
</script>