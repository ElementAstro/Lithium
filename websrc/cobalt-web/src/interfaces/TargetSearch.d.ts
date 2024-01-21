declare interface IDSOObjectInfo {
    name: string,
    ra: number,
    dec: number,
}


declare interface IDSOFramingObjectInfo{
    name: string,
    ra: number,
    dec: number,
    rotation: number,
    flag: string,   // flag is user editable, any string is ok
    tag: string,  // tag is system set filters.
    target_type: string,
    bmag: number,
    vmag: number,
    size: number,
    checked: boolean,
}


declare interface IDSOObjectDetailedInfo {
    name: string,
    ra: number,
    dec: number,
    target_type: string,
    bmag: number,
    vmag: number,
    size: number,
    moon_distance: number,
    altitude: Array<[string, number, number]>
}


declare interface ILightStarInfo {
    name: string,
    show_name: string,
    ra: number,
    dec: number,
    Const: string,
    Const_Zh: string,
    magnitude: number,
    alt: number,
    az: number,
    sky: string,
}


declare interface ITwilightDataString {
    evening: {
        sun_set_time: string,
        evening_civil_time: string,
        evening_nautical_time: string,
        evening_astro_time: string,
    },
    morning: {
        sun_rise_time: string,
        morning_civil_time: string,
        morning_nautical_time: string,
        morning_astro_time: string,
    }
}

declare interface ITwilightData {
    evening: {
        sun_set_time: Date,
        evening_civil_time: Date,
        evening_nautical_time: Date,
        evening_astro_time: Date,
    },
    morning: {
        sun_rise_time: Date,
        morning_civil_time: Date,
        morning_nautical_time: Date,
        morning_astro_time: Date,
    }
}

// interface from the api

declare interface IOFRequestLightStar {
    sky_range?: Array<string>,
    max_mag?: number,
}

declare interface IOFResponseLightStar {
    success: boolean;
    data: Array<ILightStarInfo>;
}

declare interface IOFResponseFindTargetName {
    success: boolean,
    data: Array<IDSOObjectDetailedInfo>,
}

declare interface IOFRequestFOVpoints {
    x_pixels: number,
    y_pixels: number,
    x_pixel_size: number,
    y_pixel_size: number,
    focal_length: number,
    target_ra: number,
    target_dec: number,
    camera_rotation: number,
}

declare interface IOFResponseFOVpoints {
    success: boolean,
    data: [
        [number, number],
        [number, number],
        [number, number],
        [number, number]
    ],
    message?: string,
}

declare interface IOFRequestFOVpointsTiles {
    x_pixels: number,
    y_pixels: number,
    x_pixel_size: number,
    y_pixel_size: number,
    focal_length: number,
    target_ra: number,
    target_dec: number,
    camera_rotation: number,
    x_tiles: number,
    y_tiles: number,
    overlap: number,
}

declare interface IOFResponseFOVpointsTiles {
    success: boolean,
    data: Array<[
        [number, number],
        [number, number],
        [number, number],
        [number, number]
    ]>,
    message?: string,
}

declare interface IOFResponseAltCurve {
    success: boolean,
    data: {
        moon_distance: number,
        altitude: Array<[string, number, number]>
    }
}

declare interface IOFResponseTwilightData {
    success: boolean,
    data: ITwilightDataString
}
// end of interface from the api