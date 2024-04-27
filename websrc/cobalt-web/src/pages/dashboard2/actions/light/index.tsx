import * as React from "react";
import {
  Card,
  Button,
  ButtonGroup,
  Modal,
  Accordion,
  AccordionHeader,
  AccordionBody,
} from "react-bootstrap";
import { GlobalStore } from "../../../../store/globalStore";
import * as AXIOSOF from "../../../../services/object_finding_api";
import * as AXIOSPAAF from "../../../../services/paa_fixed_procedure_api";

import LightStartSelection from "./LightStarSelection";
import ConfirmDialog from "./ConfirmDialog";

const Star = () => {
  const [east_selected, set_east_selected] = React.useState(true);
  const [west_selected, set_west_selected] = React.useState(true);
  const [south_selected, set_south_selected] = React.useState(true);
  const [north_selected, set_north_selected] = React.useState(true);
  const [all_star_lists, set_all_star_lists] = React.useState<
    [
      Array<ILightStarInfo>,
      Array<ILightStarInfo>,
      Array<ILightStarInfo>,
      Array<ILightStarInfo>
    ]
  >([[], [], [], []]);
  const [selected_star_info, set_selected_star_info] =
    React.useState<ILightStarInfo | null>(null);
  const [can_start_paa, set_can_start_paa] = React.useState(false);
  const [dialog_open, set_dialog_open] = React.useState(0);
  const [select_star_open, set_select_star_open] = React.useState(false);
  // store
  const set_console_state = GlobalStore.actions.console.setState;

  const on_filter_button_clicked = (clicked_type: string) => {
    switch (clicked_type) {
      case "east": {
        set_east_selected(!east_selected);
        get_light_star_data(
          !east_selected,
          west_selected,
          south_selected,
          north_selected
        );
        break;
      }
      case "west": {
        set_west_selected(!west_selected);
        get_light_star_data(
          east_selected,
          !west_selected,
          south_selected,
          north_selected
        );
        break;
      }
      case "north": {
        set_north_selected(!north_selected);
        get_light_star_data(
          east_selected,
          west_selected,
          south_selected,
          !north_selected
        );
        break;
      }
      case "south": {
        set_south_selected(!south_selected);
        get_light_star_data(
          east_selected,
          west_selected,
          !south_selected,
          north_selected
        );
        break;
      }
      default: {
        break;
      }
    }
  };
  const get_light_star_data = async (
    east: boolean,
    west: boolean,
    south: boolean,
    north: boolean
  ) => {
    let sky_range = [];
    if (east) {
      sky_range.push("east");
    }
    if (west) {
      sky_range.push("west");
    }
    if (south) {
      sky_range.push("north");
    }
    if (north) {
      sky_range.push("south");
    }
    let to_seach_star_request: IOFRequestLightStar = {
      sky_range: sky_range,
      max_mag: 2,
    };
    try {
      const get_light_star_data = await AXIOSOF.getLightStars(
        to_seach_star_request
      );
      /*
                self.name = name
                self.show_name = show_name
                self.ra = ra
                self.dec = dec
                self.Const = Const
                self.Const_Zh = Const_Zh
                self.magnitude = magnitude
                self.alt = None
                self.az = None
                self.sky = ''
            */
      let west_list = [];
      let east_list = [];
      let north_list = [];
      let south_list = [];
      for (let i = 0; i < get_light_star_data.data.length; i++) {
        if (get_light_star_data.data[i].sky == "west") {
          west_list.push(get_light_star_data.data[i]);
        }
        if (get_light_star_data.data[i].sky == "east") {
          east_list.push(get_light_star_data.data[i]);
        }
        if (get_light_star_data.data[i].sky == "north") {
          north_list.push(get_light_star_data.data[i]);
        }
        if (get_light_star_data.data[i].sky == "south") {
          south_list.push(get_light_star_data.data[i]);
        }
      }
      set_all_star_lists([east_list, west_list, south_list, north_list]);
    } catch (err) {
      return null;
    }
  };
  const on_star_selected = (
    star_area_index: number,
    star_array_index: number
  ) => {
    set_selected_star_info(all_star_lists[star_area_index][star_array_index]);
    set_select_star_open(false);
  };
  const update_paa_status_and_clean = async () => {
    set_can_start_paa(false);
    try {
      let paa_running_status = await AXIOSPAAF.get_update_paa_running_status();
      if (paa_running_status.data) {
        // true means in running
        set_can_start_paa(false);
      } else {
        set_can_start_paa(true);
      }
    } catch (err) {
      return null;
    }
  };
  const get_paa_status_after_start = async () => {
    set_can_start_paa(false);
    try {
      let paa_status_check = await AXIOSPAAF.get_paa_status();
      if (paa_status_check.data.flag) {
        // true meas in running
        set_can_start_paa(false);
      } else {
        set_can_start_paa(true);
      }
    } catch (err) {
      return null;
    }
  };
  const on_start_goto_clicked = async () => {
    if (selected_star_info != null) {
      try {
        let start_result = await AXIOSPAAF.start_fixed_Goto({
          ra: selected_star_info.ra,
          dec: selected_star_info.dec,
        });
        if (start_result.success) {
          // successfully started
          set_can_start_paa(false);
          set_console_state({
            alert_type: "success",
            alert_message: "亮星Goto，执行中",
          });
        } else {
          console.log(start_result.message);
          get_paa_status_after_start();
        }
      } catch (err) {
        return null;
      }
    } else {
      set_console_state({
        alert_type: "danger",
        alert_message: "亮星Goto，必须选择一个目标才能执行！",
      });
    }
  };
  const on_start_goto_center_clicked = async () => {
    if (selected_star_info != null) {
      try {
        let start_result = await AXIOSPAAF.start_fixed_Goto_Center({
          ra: selected_star_info.ra,
          dec: selected_star_info.dec,
        });
        if (start_result.success) {
          // successfully started
          set_can_start_paa(false);
          set_console_state({
            alert_type: "success",
            alert_message: "亮星Goto居中，执行中",
          });
        } else {
          console.log(start_result.message);
          get_paa_status_after_start();
        }
      } catch (err) {
        return null;
      }
    } else {
      set_console_state({
        alert_type: "danger",
        alert_message: "亮星Goto居中，必须选择一个目标才能执行！",
      });
    }
  };
  const post_stop_paa = async () => {
    try {
      let stop_result = await AXIOSPAAF.post_stop_paa();
      if (stop_result.success) {
        set_can_start_paa(true);
        update_paa_status_and_clean();
      } else {
        set_can_start_paa(true);
        console.log(stop_result.message);
      }
    } catch (err) {
      return null;
    }
  };
  const on_stop_paa_clicked = () => {
    set_dialog_open(dialog_open + 1);
  };

  React.useEffect(() => {
    set_east_selected(true);
    set_west_selected(true);
    set_north_selected(true);
    set_south_selected(true);
    get_light_star_data(true, true, true, true);
    update_paa_status_and_clean();
  }, []);

  return (
    <div style={{ zIndex: 20 }}>
      <Card>
        <Card.Body>
          <Card.Title className="text-success">
            {selected_star_info?.show_name || "未选择"}
          </Card.Title>
          <Card.Text className="text-success">
            {selected_star_info
              ? `${selected_star_info.alt.toFixed(1)}°, ${
                  selected_star_info.Const_Zh
                }`
              : "~"}
          </Card.Text>
        </Card.Body>
        <Card.Footer>
          <Button
            size="sm"
            variant="primary"
            onClick={() => set_select_star_open(true)}
          >
            选择亮星
          </Button>
        </Card.Footer>
      </Card>
      <hr />
      <LightStartSelection
        all_star_lists={all_star_lists}
        on_star_selected={on_star_selected}
        open={select_star_open}
        handleClose={() => set_select_star_open(false)}
      />
      <hr />
      {can_start_paa ? (
        <ButtonGroup color="primary" size="sm">
          <Button onClick={on_start_goto_clicked}>赤道仪指向</Button>
          <Button onClick={on_start_goto_center_clicked}>
            赤道仪指向并居中
          </Button>
        </ButtonGroup>
      ) : (
        <Button variant="danger" onClick={on_stop_paa_clicked} size="sm">
          停止PAA流程
        </Button>
      )}
      <ConfirmDialog
        open={dialog_open}
        show_text="请确认是否立即终止现在正在执行的PAA流程"
        show_title="确认终止流程"
        on_confirm_clicked={post_stop_paa}
        on_cancel_clicked={() => {}}
      />
    </div>
  );
};
// 加一个selected star。

Star.displayName = "亮星";

export default Star;
