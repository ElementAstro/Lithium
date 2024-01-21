import * as React from 'react';
import { Container, Row, Col, Button, ButtonGroup, Dropdown } from 'react-bootstrap';
import { GlobalStore } from '../../store/globalStore';
import { useEchoWebSocket } from '../../utils/websocketProvider';

interface FilterControlProps {
  detailed: boolean;
}

interface FilterInfo {
  slot: number;
  filter_name: string;
}

const DeviceFilterGeneralControlPanel: React.FC = () => {
  const [current_filter, set_current_filter] = React.useState(1);
  const [select_filter, set_select_filter] = React.useState(1);
  const [max_filter, set_max_filter] = React.useState(8);
  const [rotate_flag, set_rotate_flag] = React.useState(true);

  const [open_dialog, set_dialog] = React.useState(false);
  const [filter_info, set_filter_info] = React.useState({
    slot_index: 0,
    filter_name: "",
  });


  // 全体名称
  const initialFilterNames = GlobalStore.useAppState((state) => state.GlobalParameterStore.get_filter_names_full) || [];
  const GlobalInfo = GlobalStore.useAppState(state=>state.GlobalParameterStore).global_parameter
  const [all_filters, setAllFilters] = React.useState(initialFilterNames);


  const handleCreateOpen = () => {
    set_dialog(true);
  };
  
  const handleCreateClose = () => {
    set_dialog(false);
  };

  const handleIndexChange = (event: any) => {
    const inputValue = event.target.value;
    // 使用正则表达式检查输入是否为数字
    if (/^\d+$/.test(inputValue)) {
      // 将输入值解析为数字并更新slot_index
      set_filter_info({
        ...filter_info,
        slot_index: parseInt(inputValue, 10),
      });
    }
    else{
      // error
    }
  };

  const handleNameChange = (event: any) => {
    set_filter_info({
      ...filter_info,
      filter_name: event.target.value,
    });
  };
  
  // ws数据传递
  const process_ws_message = (msg: any): void => {
    let response_data = msg;
    console.log(response_data);
    if (response_data.device_name == 'filter'){
      switch (response_data.instruction) {
        // 获取信息
        case 'get_filter_slot': {
          // 设置最大滤镜轮数量
          set_current_filter(response_data.data.filter_slot);
          set_rotate_flag(response_data.data.idle);
          break;
        }
        case 'get_all_params': {
          set_current_filter(response_data.data.filter_slot);
          updateFiltersInfo(response_data.data.filters_info);
          break;
        }
        case 'update_all_slot_data': {
          // 没有错误返回
        }
        case 'set_one_filter_solt_value': {
          // 没有错误返回
        }
        case 'switch_filter': {
          if(response_data.message == "Filter wheel in rotating!"){
            // report error
          }
          if(response_data.message == "filter slot index value out of range!"){
            // report error
          }
        }
        default: {

        }
      }
    }
  }
  
  const {sendMessage, removeListener} = useEchoWebSocket(process_ws_message);
  
  // 更新
  const updateFiltersInfo = (filtersInfo: FilterInfo[]) => {
    const updatedFilterNames = filtersInfo.map((filterInfo) => ({
      label: filterInfo.filter_name,
      value: filterInfo.slot,
    }));
    setAllFilters(updatedFilterNames);
    set_max_filter(filtersInfo.length);
  };

  // 更换单个滤镜轮信息
  const set_filter_name = () => {
    sendMessage(JSON.stringify({
      device_name: 'filter',
      instruction: 'set_one_filter_solt_value',
      params: [filter_info.slot_index, {filter_name: filter_info.filter_name}],
    }))
    sendMessage(JSON.stringify({
      device_name: 'filter',
      instruction: 'get_all_params',
      params: [],
    }))
  }

  // 更新所有滤镜轮信息
  const update_all_filter = () => {
    const params = all_filters.map(filter => ({filter_name: filter.label}));
    sendMessage(JSON.stringify({
      device_name: 'filter',
      instruction: 'update_all_slot_data',
      params: [params],
    }))
    sendMessage(JSON.stringify({
      device_name: 'filter',
      instruction: 'get_all_params',
      params: [],
    }))
  };

  // 旋转当前滤镜轮
  const switch_filter = (slot_index: number, need_focuser: boolean) => {
    sendMessage(JSON.stringify({
      device_name: 'filter',
      instruction: 'switch_filter',
      params: [slot_index],
    }))
    // 需要focuser
    if(need_focuser){
      if(GlobalInfo.autofocus?.use_filter_offset){
        if( GlobalInfo.filter_setting?.filter_info[slot_index - 1] && GlobalInfo.filter_setting?.filter_info[current_filter - 1]){
          let to_set_move_rel_step = GlobalInfo.filter_setting.filter_info[slot_index - 1].focus_offset - GlobalInfo.filter_setting.filter_info[current_filter - 1].focus_offset
          sendMessage(JSON.stringify({
            device_name: 'focus',
            instruction: 'move_relative_step',
            params: [to_set_move_rel_step],
          }))
        }
        else{
          alert("filter not found")
        }
      }
      else{
        alert("auto focus prohibited!")
      }
    }
    // set_current_filter(slot_index);
  };

  
  const MINUTE_MS = 1000;
  React.useEffect(() => {
    // This will fire only on mount.
    sendMessage(JSON.stringify({
      device_name: 'filter',
      instruction: 'get_all_params',
      params: [],
    }))
    const interval = setInterval(() => {
      console.log('Logs every second');
      sendMessage(JSON.stringify({
        device_name: 'filter',
        instruction: 'get_filter_slot',
        params: [],
      }))
    }, MINUTE_MS);

    return () => {
      console.log('clear interval');
      clearInterval(interval);
      removeListener(process_ws_message);
    } // This represents the unmount function, in which you need to clear your interval to prevent memory leaks.
  }, [])

  return (
      <Container fluid>
        <Row className="mb-3">
          <Col md={6}>
            <div className="p-3 bg-white">
              <h4>滤镜轮信息</h4>
              <p>当前滤镜编号: {current_filter}</p>
              <p>总滤镜数量: {max_filter}</p>
              <p>运行状态: {rotate_flag ? '空闲' : '正在运转'}</p>
            </div>
          </Col>
          <Col md={6}>
            <div className="p-3 bg-white">
            <Dropdown>
                <Dropdown.Toggle variant="secondary" id="dropdown-basic">
                  {select_filter}
                </Dropdown.Toggle>

                <Dropdown.Menu>
                  {all_filters.map((filter, index) => (
                    <Dropdown.Item
                      key={index}
                      onSelect={() => {
                        let selected_value = filter;
                        if (typeof selected_value === 'string') {
                          selected_value = parseInt(selected_value, 10);
                        }
                        set_select_filter(selected_value);
                      }}
                    >
                      {filter}
                    </Dropdown.Item>
                  ))}
                </Dropdown.Menu>
              </Dropdown>
              <div className="my-1">
                <ButtonGroup vertical>
                  <Button
                    onClick={() => {
                      switch_filter(select_filter, false);
                    }}
                  >
                    切换滤镜
                  </Button>

                  <Button
                    onClick={() => {
                      switch_filter(select_filter, true);
                    }}
                  >
                    切换滤镜并移动调焦器
                  </Button>
                </ButtonGroup>
              </div>
            </div>
          </Col>
        </Row>
      </Container>
  )
}

export default DeviceFilterGeneralControlPanel;