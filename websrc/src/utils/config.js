import { Dialog } from 'quasar';

const config = {
  globalConfig: {
    name: "Quasar",
    version: "v2.0",
    theme: "dark",
    features: ["responsive", "PWA", "SSR"],
  },
  defaults: {
    theme: "light",
    fontSize: "medium",
  },
  loadFromFile(filePath) {
    const xhr = new XMLHttpRequest();
    xhr.onload = () => {
      try {
        const jsonData = JSON.parse(xhr.responseText);
        this.globalConfig = { ...this.globalConfig, ...jsonData };
        console.log(`Loaded config from file '${filePath}'.`);
      } catch (error) {
        Dialog.create({
          title: 'Error',
          message: `Failed to load config from file '${filePath}'. ${error}`,
        });
      }
    };
    xhr.onerror = (error) => {
      Dialog.create({
        title: 'Error',
        message: `Failed to load config from file '${filePath}'. ${error}`,
      });
    };
    xhr.open('GET', filePath);
    xhr.send();
  },
  get(key) {
    return this.globalConfig[key];
  },
  set(key, value) {
    try {
      // 更新全局变量
      this.globalConfig[key] = value;
      // 触发对应 key 的回调函数
      if (this.callbacks[key]) {
        this.callbacks[key].forEach(callback => callback(value));
      }
      // 更新本地存储
      localStorage.setItem("globalConfig", JSON.stringify(this.globalConfig));
      console.log(`Set config '${key}' to '${value}'.`);
    } catch (error) {
      Dialog.create({
        title: 'Error',
        message: `Failed to set config '${key}' to '${value}'. ${error}`,
      });
    }
  },
  search(key) {
    return this.globalConfig.hasOwnProperty(key);
  },
  // 监听全局变量的变化
  callbacks: {},
  on(key, callback) {
    if (!this.callbacks[key]) {
      this.callbacks[key] = [];
    }
    this.callbacks[key].push(callback);
    console.log(`Added callback for config '${key}'.`);
  },
  off(key, callback) {
    if (this.callbacks[key]) {
      this.callbacks[key] = this.callbacks[key].filter(
        existingCallback => existingCallback !== callback,
      );
      console.log(`Removed callback for config '${key}'.`);
    }
  },
  // 从本地存储中读取配置信息
  loadFromLocalStorage() {
    try {
      const savedConfig = localStorage.getItem("globalConfig");
      if (savedConfig) {
        this.globalConfig = JSON.parse(savedConfig);
        console.log(`Loaded config from local storage.`);
      } else {
        // 如果本地存储为空，则加载默认值
        Object.keys(this.defaults).forEach(key => {
          if (!this.search(key)) {
            this.set(key, this.defaults[key]);
          }
        });
      }
    } catch (error) {
      console.error(`Failed to load config from local storage. ${error}`);
    }
  },
  // 将当前的配置信息保存到本地存储中
  saveToLocalStorage() {
    try {
      localStorage.setItem("globalConfig", JSON.stringify(this.globalConfig));
      console.log("Config saved to local storage.");
    } catch (error) {
      console.error(`Failed to save config to local storage. ${error}`);
    }
  },
  // 判断全局变量是否为空
  isEmpty() {
    return Object.keys(this.globalConfig).length === 0;
  },
  // 获取全局变量的类型
  getType(key) {
    return Object.prototype.toString.call(this.globalConfig[key]).slice(8, -1);
  },
};

export default config;
