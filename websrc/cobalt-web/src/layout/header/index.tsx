import {useCallback, useState} from 'react';
import {ReactComponent as ListIcon} from '@/icons/list.svg';
import {GlobalStore} from '@/store/globalStore';
import { IconButton, Button, ButtonGroup  } from '@mui/joy';
import QuickControlComp from './QuickControlComp';
import horivisison_logo from '../../assets/imgs/logo-title.png';

import FormatListBulletedIcon from '@mui/icons-material/FormatListBulleted';



import './style.less';

const Header = ()=>{
    const {drawerVisible} = GlobalStore.useAppState(state=>state.visible);

    const toggleNavigation = useCallback(()=>{
        GlobalStore.actions.visible.setState({
            drawerVisible:!drawerVisible
        })
    },[drawerVisible]);

    return (
        <div className='app-drawer-btn'>
            <IconButton variant="outlined" size='sm' color="primary" onClick={toggleNavigation} sx={{zIndex: 100}}>
                <FormatListBulletedIcon />
            </IconButton>  
            <img className='app-drawer-btn-logo' alt="logo" id="logo" src={horivisison_logo}></img>
            {/* <span className='app-drawer-btn-title'>HORIVISION</span> */}
            <QuickControlComp />
        </div>
        // <div className='app-header'>
        //     <ListIcon fill="#fff" onClick={toggleNavigation}/>
        //     <span className='app-header-title'>LightGPT</span>
        // </div>
    )
}

export default Header;