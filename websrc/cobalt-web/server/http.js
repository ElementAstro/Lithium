import { createServer } from "http";
import { parse } from "url";

// 创建 HTTP 服务器
const server = createServer((req, res) => {
  // 解析请求 URL
  const parsedUrl = parse(req.url, true);

  // 获取请求路径
  const path = parsedUrl.pathname;

  // 获取请求方法
  const method = req.method;

  // 设置响应头
  res.setHeader("Content-Type", "application/json");

  // 处理不同的接口
  if (path === "/GlobalParameter/" && method === "GET") {
    // 处理 /api/data GET 请求
    const responseData = {
      message: "Hello, this is your data!",
      data: {
        meridian_flip: {
          before_meridian: 2,
          after_meridian: 2,
          recenter_after_flip: true,
          autofocus_after_flip: false,
          settle_time_after: 30,
        },
        guider_start_guide_settle: {
          time: 30,
          timeout: 300,
          pixels: 1.5,
        },
        guider_dither_settle: {
          time: 5,
          timeout: 30,
          pixels: 1.5,
        },
        filter_setting: {
          filter_number: 8,
          filter_info: [
            {
              filter_name: "filter1",
              focus_offset: 0,
              af_exposure_time: 1,
            },
            {
              filter_name: "filter2",
              focus_offset: 0,
              af_exposure_time: 1,
            },
          ],
        },
        autofocus: {
          use_filter_offset: true,
          step_size: 50,
          initial_steps: 5,
          default_exposure_time: 1,
          retry_times: 1,
          each_step_exposure: 1,
          af_after_filter_change: false,
          af_offset_base_filter_slot: 0,
        },
        plate_solve: {
          use: "astap",
          exposure_time: 2,
          use_filter: "Current",
          downsample: 4,
          tolerance: 100,
        },
        telescope_info: {
          name: "",
          aperture: 80,
          focal_length: 480,
          guider_aperture: 60,
          guider_focal_length: 240,
        },
        info_get: {
          camera: false,
          guider_camera: false,
        },
        camera_info: {
          CCD_MAX_X: 0,
          CCD_MAX_Y: 0,
          CCD_PIXEL_SIZE: 0,
        },
        guider_camera_info: {
          CCD_MAX_X: 0,
          CCD_MAX_Y: 0,
          CCD_PIXEL_SIZE: 0,
        },
        dither: {
          amount: 2,
          ra_only: false,
        },
      },
    };
    res.statusCode = 200;
    res.end(JSON.stringify(responseData));
  } else if (path === "/api/postData" && method === "POST") {
    // 处理 /api/postData POST 请求
    let body = "";
    req.on("data", (chunk) => {
      body += chunk.toString();
    });
    req.on("end", () => {
      const postData = JSON.parse(body);
      const responseData = {
        message: "Data received successfully!",
        receivedData: postData,
      };
      res.statusCode = 200;
      res.end(JSON.stringify(responseData));
    });
  } else {
    // 处理未知路由
    res.statusCode = 404;
    res.end(JSON.stringify({ error: "Not Found" }));
  }
});

// 监听端口
const port = 3000;
server.listen(port, () => {
  console.log(`Server is running on port ${port}`);
});
