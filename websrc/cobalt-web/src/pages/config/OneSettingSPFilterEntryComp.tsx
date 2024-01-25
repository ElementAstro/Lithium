import React, { useState } from "react";
import { Form, Row, Col } from "react-bootstrap";
import { XCircle } from "react-bootstrap-icons";

import GlobalParameterFilterSetting from "./OneSettingSPSingleFilterComp";
import GPRedDotComp from "./ClickQuestionTooltips";

interface AllFilterSettingProps {
  setting_part_name: string;
  setting_part_value: IFilterSetting;
  tooltip?: string;
  handleSettingChange: (
    setting_part_key: string,
    setting_value: Partial<IGlobalParameters>
  ) => void;
  parameter_explain: IGPParameterExplain;
}

const GlobalParameterAllFilterSettings: React.FC<AllFilterSettingProps> = (
  props
) => {
  const [error, setError] = useState(false);
  const [helperText, setHelperText] = useState(props.tooltip);
  const [showChanging, setShowChanging] = useState(false);
  const [showFilterNumber, setShowFilterNumber] = useState(
    props.setting_part_value.filter_number
  );

  // handle change
  const handleFilterSettingChange = (
    filterNumber: number,
    settingValue: ISingleFilterInfo
  ) => {
    let new_filter_info = props.setting_part_value;
    new_filter_info.filter_info[filterNumber] = settingValue;
    // 单个组件进行了数据修改
    props.handleSettingChange(props.setting_part_name, new_filter_info);
  };

  const handleFilterNumberChange = () => {
    let new_filter_number: number = showFilterNumber;
    let to_set_filter_info = props.setting_part_value;
    if (new_filter_number < 0) {
      new_filter_number = 0;
    }
    if (new_filter_number > 12) {
      new_filter_number = 12;
    }
    // 修改了滤镜数量
    if (new_filter_number !== to_set_filter_info.filter_number) {
      if (new_filter_number > to_set_filter_info.filter_number) {
        console.log(to_set_filter_info.filter_number, new_filter_number);
        // 新增
        for (
          let i = to_set_filter_info.filter_number;
          i < new_filter_number;
          i++
        ) {
          const default_empty_single_focus_data: ISingleFilterInfo = {
            filter_name: "Filter X",
            focus_offset: 0,
            af_exposure_time: 1,
          };
          to_set_filter_info.filter_info.push(default_empty_single_focus_data);
        }
      } else {
        // 减少
        to_set_filter_info.filter_info = to_set_filter_info.filter_info.slice(
          0,
          new_filter_number
        );
      }
      to_set_filter_info.filter_number = new_filter_number;
      props.handleSettingChange(props.setting_part_name, to_set_filter_info);
    }
  };

  // render different components
  const renderFilterSetting = (
    settingKey: string,
    settingValue: number | ISingleFilterInfo[]
  ) => {
    if (settingKey === "filter_number") {
      return (
        <Row className="mt-1">
          <Col xs={10}>
            <Form.Control
              size="sm"
              type="number"
              title={props.parameter_explain[settingKey]?.name}
              value={showFilterNumber}
              min={0}
              max={12}
              step={1}
              isInvalid={error}
              onChange={(event) => {
                setShowChanging(true);
                setShowFilterNumber(parseInt(event.target.value));
              }}
              onBlur={() => {
                handleFilterNumberChange();
                setShowChanging(false);
              }}
            />
            <Form.Control.Feedback type="invalid">
              {helperText}
            </Form.Control.Feedback>
          </Col>
          <Col xs={2}>
            <GPRedDotComp
              show_changing={showChanging}
              tooltip={props.tooltip}
            />
          </Col>
        </Row>
      );
    } else if (settingValue instanceof Array) {
      return (
        <GlobalParameterFilterSetting
          filters_info={settingValue}
          handle_single_filter_setting={handleFilterSettingChange}
          parameter_explain={props.parameter_explain}
        />
      );
    } else {
      return <div>* filter 出错啦 *</div>;
    }
  };

  return (
    <div>
      {Object.entries(props.setting_part_value).map(([settingKey, setValue]) =>
        renderFilterSetting(settingKey, setValue)
      )}
    </div>
  );
};

export default GlobalParameterAllFilterSettings;
