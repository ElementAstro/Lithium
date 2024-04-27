import * as React from "react";
import { useEchoWebSocket } from "../../utils/websocketProvider";
import { GlobalStore } from "../../store/globalStore";
import { setInterval } from "timers/promises";

const WS_LISTENER_COMP: React.FC = () => {
  // store taker
  const register_send_message =
    GlobalStore.actions.ProcessDataSaveStore.at_start_add_ws_listener;
  // const send_ws_message = GlobalStore.actions.ProcessDataSaveStore.send_message_to_ws;
  const process_data = GlobalStore.useAppState(
    (state) => state.ProcessDataSaveStore
  );
  const append_HFR =
    GlobalStore.actions.ProcessDataSaveStore.append_newest_history_HFR_data;
  const append_guide_step =
    GlobalStore.actions.ProcessDataSaveStore.append_newest_PHD2_guide_data;
  const get_newest_jpg =
    GlobalStore.actions.ProcessDataSaveStore.get_newest_jpg;
  const load_saved_targets =
    GlobalStore.actions.TargetListStore.load_all_targets;
  const initial_ui_setting =
    GlobalStore.actions.GlobalParameterStore.initial_ui_setting;
  const load_ui_setting =
    GlobalStore.actions.GlobalParameterStore.load_ui_setting;
  const get_all_global_parameters =
    GlobalStore.actions.GlobalParameterStore.get_all_paramters;

  const process_ws_message = (message: any) => {
    // listening signals and other important data
    if (message.device_name == "Signal") {
      if (message.instruction == "Image Stretch") {
        if (message.data.camera_type == "camera") {
          get_newest_jpg("camera");
          // console.log('camera got stretched!', typeof sendMessageRef.current);
          append_HFR(message.data.hfr_info);
        } else if (message.data.camera_type == "guider") {
          get_newest_jpg("guider");
        }
      } else if (message.instruction == "Image Process Failed!") {
      }
    } else if (message.device_name == "phd2") {
      // message from server
      switch (message.instruction) {
        case "get_guiding_data": {
          GlobalStore.actions.ProcessDataSaveStore.setState({
            PHD2_guide_data_list: {
              dx: message.data.dx,
              dy: message.data.dy,
              RaControl: message.data.RaControl,
              DecControl: message.data.DecControl,
              RaDistance: message.data.RaDistance,
              DecDistance: message.data.DecDistance,
            },
          });
          break;
        }
        case "get_record_guide_rms": {
          break;
        }
      }
    } else if (message.device_name == "PHD2_event") {
      // message from phd2
      console.log("phd2 event", message.message);
      if (message.message == "GuideStep") {
        // process newest guide step data
        append_guide_step(message.data);
      } else if (message.message == "get_calibration_data") {
        GlobalStore.actions.ProcessDataSaveStore.setState({
          PHD2_calibration_result: message.data,
        });
      }
    } else if (message.device_name == "PHD2_response") {
    } else if (message.device_name == "PAA") {
      if (message.type == "step") {
        GlobalStore.actions.PAA.paa_new_step_info_in(message);
      } else if (message.instruction == "log") {
        GlobalStore.actions.PAA.paa_new_log_info_in(message);
      }
    }
  };
  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);
  // useEffect
  React.useEffect(() => {
    load_saved_targets();
    initial_ui_setting();
    load_ui_setting();
    register_send_message(sendMessage);
    get_all_global_parameters();
    // get history guiding data
    // todo fatal bug, this sendMessage is not available at this step.
    setTimeout(() => {
      // console.log(sendMessage);
      sendMessage(
        JSON.stringify({
          device_name: "phd2",
          instruction: "get_guiding_data",
          params: [],
        })
      );
    }, 500);
    // get history hfr data, todo
  }, []);
  // 这个listener只添加一次，不能删除
  return <></>;
};
/* 
现在的问题是，我需要有在全局有且仅有一次注册一个websocket的listener，去对一些特殊信号进行处理。
同时，有一部分信号要去ws接口里去send message。
然后这个listener获取的数据，都要在一个store里面更新。
所以这个功能怎么放比较好，又要只注册一次listener，又要能访问store的功能。
想了一下在一个store里面写，碰到的问题是，如何在store内部调用websocket注册一个listener，这个listener又能访问store的state去修改数据？或者能访问store的actions。没找到具体写法
在一个欢迎界面下写组件，别的问题没有，怎么解决仅注册这一次？
*/

export default WS_LISTENER_COMP;
