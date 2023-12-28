import axios from "axios";
import config from "../constants/net-config";

let tokenLose = true;

const { baseURL, successCode, invalidCode, requestTimeout, contentType } =
  config;

const instance = axios.create({
  baseURL,
  timeout: requestTimeout,
  headers: {
    "Content-Type": contentType,
  },
});

// 请求拦截器
instance.interceptors.request.use(
  (configItem) => configItem,
  (error) => Promise.reject(error)
);

// 响应拦截器
instance.interceptors.response.use(
  (response) => {
    const res = response.data;
    // 处理错误情况
    if (res.code === invalidCode && tokenLose) {
      tokenLose = false;
      // 根据你的业务需求进行相应处理
    }

    if (successCode.indexOf(res.code) === -1) {
      console.error(res.msg);
      return Promise.reject(res);
    }

    return res;
  },
  (error) => {
    console.error("请求出错了！");
    return Promise.reject(error);
  }
);

export const request = {
  /**
   * 发起get请求
   * @param {string} url 请求的url
   * @param {object} params 请求参数
   * @returns {Promise}
   */
  get(url, params) {
    return instance.get(url, { params });
  },

  /**
   * 发起post请求
   * @param {string} url 请求的url
   * @param {object} data 请求数据
   * @returns {Promise}
   */
  post(url, data) {
    return instance.post(url, data);
  },

  /**
   * 发起put请求
   * @param {string} url 请求的url
   * @param {object} data 请求数据
   * @returns {Promise}
   */
  put(url, data) {
    return instance.put(url, data);
  },

  /**
   * 发起delete请求
   * @param {string} url 请求的url
   * @returns {Promise}
   */
  delete(url) {
    return instance.delete(url);
  },
};

export default instance;
