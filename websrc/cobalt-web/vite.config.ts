import { ConfigEnv, UserConfigExport } from "vite";
import react from "@vitejs/plugin-react";
import svgr from "vite-plugin-svgr";
import path from "path";
// import { viteMockServe } from 'vite-plugin-mock'

const resolve = (dir: string) => path.join(__dirname, ".", dir);

const isDev = process.env.NODE_ENV === "development";

export default ({ command }: ConfigEnv): UserConfigExport => {
  return {
    publicDir: "public",
    plugins: [
      react(),
      svgr(),
      /*viteMockServe({
      mockPath: 'mock',
      localEnabled: false,
      supportTs: true,
      watchFiles: true,
    }),*/
    ],
    resolve: {
      alias: {
        "@": resolve("src"),
      },
    },
    css: {
      preprocessorOptions: {
        less: {
          javascriptEnabled: true,
        },
      },
    },
    server: {
      cors: true,
      watch: {
        usePolling: true,
      },
      // http://192.168.31.193:7999
      proxy: {
        "^/api/.*": {
          target: "http://192.168.31.193:7999",
          changeOrigin: true,
          ws: true,
          rewrite: (path) => path.replace(/^\/api/, ""),
        },
      },
    },
  };
};
