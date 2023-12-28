declare interface IRequestParams {
    url:string;
    method?:"GET"|"POST",
    body?:any
}

declare interface IRequestResponse<T> {
    success: boolean;
    data?: T;
    message?: string;
}