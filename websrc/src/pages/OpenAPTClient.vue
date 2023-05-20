以下是修改后的代码：

<template>
  <div>
    <q-splitter v-model="splitterModel" style="height: 100%">
      <template v-slot:before>
        <q-tabs v-model="tab" class="text-teal bg-white shadow-2" vertical>
          <q-tab name="dashboard" icon="fas fa-home"></q-tab>
          <q-tab name="control" icon="fas fa-gear">
          </q-tab>
        </q-tabs>
      </template>

      <template v-slot:after>
        <q-tab-panels v-model="tab" animated swipeable transition-prev="jump-up" transition-next="jump-up">
          <q-tab-panel name="dashboard">
            <dashboard-component :data="dashboardData" @send-message="sendMessage" ref="dashboardComponent" />
          </q-tab-panel>

          <q-tab-panel name="control">
            <q-splitter v-model="splitterModel" style="height: 100%">
              <template v-slot:before>
                <q-tabs v-model="tab" class="text-teal bg-white shadow-2" vertical>
                  <q-tab name="camera" icon="fas fa-camera" label="Camera"></q-tab>
                  <q-tab name="mount" icon="fas fa-mountain" label="Mount"></q-tab>
                  <q-tab name="focuser" icon="fas fa-adjust" label="Focuser"></q-tab>
                  <q-tab name="filterWheel" icon="fas fa-filter" label="Wheel"></q-tab>
                  <q-tab name="solver" icon="fas fa-search" label="Solver"></q-tab>
                  <q-tab name="search" icon="fas fa-search" label="Search"></q-tab>
                </q-tabs>
              </template>

              <template v-slot:after>
                <q-tab-panels v-model="tab" animated swipeable transition-prev="jump-up" transition-next="jump-up">
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
                    <filter-wheel-component :data="filterWheelData" @send-message="sendMessage"
                      ref="filterWheelComponent" />
                  </q-tab-panel>

                  <q-tab-panel name="solver">
                    <solver-component :data="solverData" @send-message="sendMessage" ref="solverComponent" />
                  </q-tab-panel>

                  <q-tab-panel name="search">
                    <search-component :data="searchData" @send-message="sendMessage" ref="searchComponent" />
                  </q-tab-panel>

                  <q-tab-panel name="console">
                    <search-component :data="consoleData" @send-message="sendMessage" ref="consoleComponent" />
                  </q-tab-panel>
                </q-tab-panels>
              </template>
            </q-splitter>

          </q-tab-panel>
        </q-tab-panels>
      </template>
    </q-splitter>

    <q-dialog v-model="errorDialog" title="Error">
      <div class="close-button">
        <q-btn round dense icon="mdi-close" @click="errorDialog = false"></q-btn>
      </div>
      <q-card class="error-dialog">
        <q-card-section class="error-dialog-section">
          <div>
            <p>Oops! Something went wrong:</p>
            <pre>{{ error }}</pre>
            <p>Please try again later or contact support if the problem persists.</p>
          </div>
        </q-card-section>
        <q-card-actions align="right">
          <q-btn label="Email" icon="fa-solid fa-envelope" color="primary" class="q-mt-sm" @click="openEmail" />
          <q-btn label="OK" icon="fa-solid fa-check" color="primary" class="q-mt-sm" @click="errorDialog = false" />
        </q-card-actions>
      </q-card>
    </q-dialog>

    <q-dialog v-model="reconnectDialog" persistent>
      <q-card>
        <q-card-section>
          <div style="display: flex; justify-content: center; align-items: center;">
            <q-spinner-dots color="primary" size="40px" />
            <p style="margin-left: 10px;">Reconnecting...</p>
          </div>
        </q-card-section>
      </q-card>
    </q-dialog>
  </div>
</template>

<script>
import { ref } from "vue";
import CameraComponent from "../components/CameraComponent.vue";
import MountComponent from "../components/MountComponent.vue";
import FocuserComponent from "../components/FocuserComponent.vue";
import FilterWheelComponent from "../components/FilterWheelComponent.vue";
import SolverComponent from "../components/SolverComponent.vue";
import SearchComponent from "../components/SearchComponent.vue";
import DashboardComponent from "../components/DashboardComponent.vue";
import ConsoleComponent from "../components/ConsoleComponent.vue";

export default {
  components: {
    "camera-component": CameraComponent,
    "mount-component": MountComponent,
    "focuser-component": FocuserComponent,
    "filter-wheel-component": FilterWheelComponent,
    "solver-component": SolverComponent,
    "search-component": SearchComponent,
    "dashboard-component": DashboardComponent
  },
  setup() {
    const dashboardData = ref({
      //...
    });
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
    const searchData = ref({
      //...
    });

    const tab = ref("dashboard");
    const splitterModel = ref(10);
    const wsClient = new WebSocket("ws://localhost:8080");
    let reconnectTimer = null;

    const error = ref("");
    const errorDialog = ref(false);

    const reconnectDialog = ref(false);

    const connected = ref(false);

    // 发送消息到WebSocket服务器
    const sendMessage = (message) => {
      if (wsClient.readyState === WebSocket.OPEN) {
        wsClient.send(JSON.stringify(message));
      } else {
        showErrorDialog();
      }
    };

    // 显示错误弹窗
    const showErrorDialog = () => {
      error.value = "Failed to send message: WebSocket connection not open.";
      errorDialog.value = true;
    };

    // 处理来自服务器的消息，根据消息类型更新对应的组件状态
    wsClient.onmessage = (event) => {
      const data = JSON.parse(event.data);
      if (data.type === "dashboard") {
        dashboardData.value = data.payload;
        this.$refs.dashboardComponent.updateState();
      } else if (data.type === "camera") {
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
      } else if (data.type === "search") {
        solverData.value = data.payload;
        this.$refs.searchComponent.updateState();
      }
    };

    // WebSocket连接失败，显示错误弹窗，并且自动尝试重连
    const handleConnectionError = () => {
      error.value = "Failed to connect to WebSocket server.";
      errorDialog.value = true;

      reconnectTimer = setTimeout(() => {
        reconnectDialog.value = true;
        wsClient.close();

        wsClient.onopen = () => {
          reconnectDialog.value = false;
          clearTimeout(reconnectTimer);
          connected.value = true;
        };

        handleConnectionError();
      }, 5000);
    };

    // WebSocket连接成功或恢复，关闭弹窗
    wsClient.onopen = () => {
      reconnectDialog.value = false;
      clearTimeout(reconnectTimer);
      connected.value = true;
    };

    // WebSocket连接失败或断开，处理错误
    wsClient.onerror = () => {
      handleConnectionError();
    };
    wsClient.onclose = () => {
      handleConnectionError();
      connected.value = false;
    };

    return {
      dashboardData,
      cameraData,
      mountData,
      focuserData,
      filterWheelData,
      solverData,
      searchData,
      tab,
      splitterModel,
      wsClient,
      sendMessage,
      error,
      errorDialog,
      reconnectDialog,
      connected
    };
  },
};
</script>