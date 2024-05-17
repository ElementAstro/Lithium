import * as React from 'react';
import KeyboardArrowRightIcon from '@mui/icons-material/KeyboardArrowRight';
import KeyboardArrowLeftIcon from '@mui/icons-material/KeyboardArrowLeft';
import Collapse from "@mui/material/Collapse";
import { Box } from '@mui/material';
import { IconButton, Button, ButtonGroup  } from '@mui/material';
import { GlobalStore } from '@/store/globalStore';


const QuickControlComp: React.FC = () => {
  const [expand, setExpand] = React.useState(false);
  const state = GlobalStore.useAppState(state=>state.console);

  const on_quick_control_click = (target: 'camera' | 'telescope' | 'focuser' | 'filter' | 'guider' | 'phd2') => {
    GlobalStore.actions.console.setState({
      menu: target,
      drawerVisible: true,
      selec_console_drawer_open: false,
    });
    window.location.hash = 'console';
  }

  return (
    <>
      <Collapse in={expand} sx={{
          position: 'absolute',
          right: 46, top: 1
      }}>
          <ButtonGroup variant="contained" size='small' color="primary" sx={{
              zIndex: 99,
          }}>
              <Button color="primary" onClick={() => on_quick_control_click('camera')}>相机</Button>
              <Button color="primary" onClick={() => on_quick_control_click('telescope')}>赤道仪</Button>
              <Button color="primary" onClick={() => on_quick_control_click('focuser')}>调焦器</Button>
              <Button color="primary" onClick={() => on_quick_control_click('filter')}>滤镜轮</Button>
              {/* <Button onClick={() => on_quick_control_click('guider')}>导星相机</Button> */}
              <Button color="primary" onClick={() => on_quick_control_click('phd2')}>PHD2</Button>
          </ButtonGroup>
      </Collapse>
      <Box  sx={{position: 'absolute', right:6, top: 1}}>
          {
              (expand)?
              (<IconButton size='small' onClick={() => setExpand(false)} color="secondary" sx={{zIndex: 99}}><KeyboardArrowRightIcon/></IconButton>):
              (<IconButton size='small' onClick={() => setExpand(true)} color="secondary" sx={{zIndex: 99}}><KeyboardArrowLeftIcon/></IconButton>)
          }
      </Box>
    </>
  )
}

export default QuickControlComp;
