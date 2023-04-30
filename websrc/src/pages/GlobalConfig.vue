<template>
    <div class="global-config">
        <div class="header">
            <img src="../assets/quasar-logo-vertical.svg" alt="Logo" class="logo" />
            <h1 class="title">{{ name }}</h1>
            <button class="toggle-btn" @click="isFolded = !isFolded">
                <i :class="['icon', isFolded ? 'icon-chevron-down' : 'icon-chevron-up']"></i>
            </button>
        </div>

        <transition name="slide">
            <div class="content" v-show="!isFolded">
                <form class="config-form">
                    <div class="form-group">
                        <label>名称：</label>
                        <p class="show-code">{{ name }}</p>
                    </div>

                    <div class="form-group">
                        <label>版本号：</label>
                        <p class="show-code">{{ version }}</p>
                    </div>

                    <div class="form-group">
                        <label>主题：</label>
                        <p class="show-code">{{ theme }}</p>
                    </div>

                    <div class="form-group">
                        <label>功能列表：</label>
                        <div class="features">
                            <div class="feature-item" v-for="feature in featureList" :key="feature.id">
                                <div class="feature-name">
                                    {{ feature.name }}
                                    <span class="show-code">{{ '{' }}</span>
                                </div>
                                <div class="sub-features">
                                    <div class="sub-feature-item" v-for="subFeature in feature.subFeatures"
                                        :key="subFeature.id">
                                        <span class="show-code">{{ subFeature.name }}</span><span class="show-code">{{ ','
                                        }}</span>
                                    </div>
                                </div>
                                <div class="feature-name">
                                    <span class="show-code">{{ '}' }}</span>
                                </div>
                            </div>
                        </div>
                    </div>
                </form>
            </div>
        </transition>
    </div>
</template>
  
<style scoped>
.global-config {
    max-width: 90vw;
    margin: 0 auto;
    border: 1px solid #ccc;
    padding: 12px;
    border-radius: 8px;
}

.header {
    display: flex;
    align-items: center;
}

.logo {
    width: 40px;
    height: 40px;
    margin-right: 12px;
}

.title {
    font-size: 24px;
    margin: 0;
}

.toggle-btn {
    background-color: transparent;
    border: none;
    outline: none;
    cursor: pointer;
    margin-left: auto;
}

.icon {
    font-size: 20px;
}

.icon-chevron-up {
    transform: rotate(180deg);
}

.slide-enter-active,
.slide-leave-active {
    transition: all 0.3s ease;
}

.slide-enter,
.slide-leave-to {
    max-height: 0;
    opacity: 0;
}

.config-form {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    grid-gap: 12px;
    margin-top: 12px;
}

.form-group {
    display: flex;
    flex-direction: column;
}

label {
    font-weight: bold;
    margin-bottom: 4px;
}

.show-code {
    font-family: "Fira Code", monospace;
    background-color: rgba(255, 228, 196, 0.2);
    border-radius: 4px;
    padding: 4px;
    margin-left: 8px;
}

.features {
    display: flex;
    flex-wrap: wrap;
    margin: -4px;
}

.feature-item {
    display: flex;
    flex-direction: column;
    margin: 4px;
    border: 1px solid #ccc;
    border-radius: 4px;
    width: 100%;
}

.sub-features {
    margin-top: 12px;
    padding-left: 20px;
}

.sub-feature-item {
    display: flex;
    align-items: center;
    margin-top: 6px;
}

.feature-name {
    display: flex;
    align-items: center;
    font-weight: bold;
}
</style>
  
<script>
import config from "../utils/config";

export default {
    name: "GlobalConfig",
    data() {
        return {
            name: "",
            version: "",
            theme: "",
            featureList: [{ id: 1, name: "", subFeatures: [] }],
            isFolded: false,
        };
    },
    created() {
        this.loadConfig();
        this.registerCallbacks();
    },
    methods: {
        loadConfig() {
            // 从本地存储中读取配置信息
            config.loadFromLocalStorage();

            // 如果全局变量为空，则从外部文件加载配置信息
            if (config.isEmpty()) {
                config.loadFromFile("./config.json");
            }

            // 获取配置信息并保存到组件中
            this.name = config.get("name");
            this.version = config.get("version");
            this.theme = config.get("theme");
            this.featureList = config.get("features");
        },
        registerCallbacks() {
            // 注册当主题配置项改变时的回调函数
            config.on("theme", (value) => {
                this.theme = value;
            });

            // 注册当功能列表配置项改变时的回调函数
            config.on("features", (value) => {
                this.featureList = value;
            });
        },
    },
};
</script>