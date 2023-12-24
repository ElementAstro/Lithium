import { isPlainObject } from "lodash-es";

export function assignDeep(object:any,otherObject:any){
    for(const k of Object.keys(otherObject)){
        if(
            !(k in object) ||
            !(isPlainObject(object[k])) && isPlainObject(otherObject[k])
        ){
            object[k] = otherObject[k]
        }else {
            assignDeep(object[k],otherObject[k])
        }
    }
}