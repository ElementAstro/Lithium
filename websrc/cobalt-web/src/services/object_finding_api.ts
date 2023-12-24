import request from '@/services/request';
import { AxiosResponse } from 'axios';


export const getLightStars = (star_filter: IOFRequestLightStar) : Promise<IOFResponseLightStar> => request({
    url: '/TargetSearch/light_star/',
    method: 'post',
    data: star_filter,
})


export const findTargetByName = (to_find_name: string) : Promise<IOFResponseFindTargetName> => request({
    url: '/TargetSearch/name/',
    method: 'post',
    data: {name: to_find_name}
})


// export const findTargetByNameWithFilter = () : Promise => request({
//     url: '/TargetSearch/name_filter/',
//     method: 'post'
// })


// export const findTargetByFilter = () : Promise => request({
//     url: '/TargetSearch/filter/',
//     method: 'post'
// })


// export const findTodaySuggest = () : Promise => request({
//     url: '/TargetSearch/today_suggest/',
//     method: 'post'
// })


export const getFovPointsOfRect = (fov_request: IOFRequestFOVpoints) : Promise<IOFResponseFOVpoints> => request({
    url: '/TargetSearch/fov_points/',
    method: 'post',
    data: fov_request
})

// this one is not available currently
export const getTileFovPointsOfRect = (fov_request: IOFRequestFOVpointsTiles): Promise<IOFResponseFOVpointsTiles> => request({
    url: '/TargetSearch/fov_points_tiles/',
    method: 'post',
    data: fov_request
})
