declare interface IDSOObjectInfo {
    name: string,
    ra: float,
    dec: float,
}


declare interface IDSOFramingObjectInfo{
    name: string,
    ra: float,
    dec: float,
    rotation: float,
    flag: string,   // flag is user editable, any string is ok
    tag: string,  // tag is system set filters.
}


declare interface IDSOObjectDetailedInfo {
    name: string,
    ra: float,
    dec: float,
    target_type: string,
    bmag: float,
    vmag: float,
    size: float,
    moon_distance: float,
    altitude: Array<[string, float, float]>
}


declare interface ILightStarInfo {
    name: string,
    show_name: string,
    ra: float,
    dec: float,
    Const: string,
    Const_Zh: string,
    magnitude: float,
    alt: float,
    az: float,
    sky: string,
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

// end of interface from the api