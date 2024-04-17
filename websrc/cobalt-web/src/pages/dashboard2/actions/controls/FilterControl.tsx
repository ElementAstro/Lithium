import * as React from "react";
import { Card } from "react-bootstrap";
import { GlobalStore } from "../../../../store/globalStore";
import { useEchoWebSocket } from "../../../../utils/websocketProvider";
import { ButtonGroup, Button } from "react-bootstrap";

interface FilterInfo {
  slot: number;
  filter_name: string;
}

const DeviceFilterSimpleControlPanel: React.FC = () => {
  const [current_filter, set_current_filter] = React.useState(0);
  const [max_filter, set_max_filter] = React.useState(8);
  const [rotate_flag, set_rotate_flag] = React.useState(true);
  // 全体名称
  const initialFilterNames =
    GlobalStore.useAppState(
      (state) => state.GlobalParameterStore.get_filter_names_full
    ) || [];
  const GlobalInfo = GlobalStore.useAppState(
    (state) => state.GlobalParameterStore
  ).global_parameter;
  const [all_filters, setAllFilters] = React.useState(initialFilterNames);

  // ws数据传递
  const process_ws_message = (msg: any): void => {
    let response_data = msg;
    console.log(response_data);
    if (response_data.device_name == "filter") {
      switch (response_data.instruction) {
        // 获取信息
        case "get_filter_slot": {
          // 设置最大滤镜轮数量
          set_current_filter(response_data.data.filter_slot);
          set_rotate_flag(response_data.data.idle);
          break;
        }
        case "get_all_params": {
          set_current_filter(response_data.data.filter_slot);
          updateFiltersInfo(response_data.data.filters_info);
          break;
        }
        case "update_all_slot_data": {
          // 没有错误返回
        }
        case "set_one_filter_solt_value": {
          // 没有错误返回
        }
        case "switch_filter": {
          if (response_data.message == "Filter wheel in rotating!") {
            // report error
          }
          if (
            response_data.message == "filter slot index value out of range!"
          ) {
            // report error
          }
        }
        default: {
        }
      }
    }
  };

  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);

  // 更新
  const updateFiltersInfo = (filtersInfo: FilterInfo[]) => {
    const updatedFilterNames = filtersInfo.map((filterInfo) => ({
      label: filterInfo.filter_name,
      value: filterInfo.slot,
    }));
    setAllFilters(updatedFilterNames);
    set_max_filter(filtersInfo.length);
  };
  const OnFilterClicked = (slot_number: number) => {
    if (slot_number != current_filter) {
      switch_filter(slot_number, true);
    }
  };
  // 旋转当前滤镜轮
  const switch_filter = (slot_index: number, need_focuser: boolean) => {
    sendMessage(
      JSON.stringify({
        device_name: "filter",
        instruction: "switch_filter",
        params: [slot_index],
      })
    );
    // 需要focuser
    if (need_focuser) {
      if (GlobalInfo.autofocus?.use_filter_offset) {
        if (
          GlobalInfo.filter_setting?.filter_info[slot_index - 1] &&
          GlobalInfo.filter_setting?.filter_info[current_filter - 1]
        ) {
          let to_set_move_rel_step =
            GlobalInfo.filter_setting.filter_info[slot_index - 1].focus_offset -
            GlobalInfo.filter_setting.filter_info[current_filter - 1]
              .focus_offset;
          sendMessage(
            JSON.stringify({
              device_name: "focus",
              instruction: "move_relative_step",
              params: [to_set_move_rel_step],
            })
          );
        } else {
          alert("filter not found");
        }
      } else {
        alert("auto focus prohibited!");
      }
    }
    // set_current_filter(slot_index);
  };
  const MINUTE_MS = 1000;
  React.useEffect(() => {
    // This will fire only on mount.
    sendMessage(
      JSON.stringify({
        device_name: "filter",
        instruction: "get_all_params",
        params: [],
      })
    );
    const interval = setInterval(() => {
      console.log("Logs every second");
      sendMessage(
        JSON.stringify({
          device_name: "filter",
          instruction: "get_filter_slot",
          params: [],
        })
      );
    }, MINUTE_MS);

    return () => {
      console.log("clear interval");
      clearInterval(interval);
      removeListener(process_ws_message);
    }; // This represents the unmount function, in which you need to clear your interval to prevent memory leaks.
  }, []);

  return (
    <Card style={{ zIndex: 20 }}>
      {all_filters.length > 0 ? (
        <ButtonGroup>
          {all_filters.map((one_filter, index) => {
            return (
              <Button
                key={index}
                variant={
                  current_filter == one_filter.value ? "danger" : "primary"
                }
                onClick={() => {
                  OnFilterClicked(one_filter.value as number);
                }}
              >
                {one_filter.label}
              </Button>
            );
          })}
        </ButtonGroup>
      ) : (
        <p className="text-warning">全局变量中没有检测到滤镜配置！</p>
      )}
    </Card>
  );
};

DeviceFilterSimpleControlPanel.displayName = "滤镜轮控制";

export default DeviceFilterSimpleControlPanel;
