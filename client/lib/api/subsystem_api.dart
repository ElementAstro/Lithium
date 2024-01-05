/// @author: cairuoyu
/// @homepage: http://cairuoyu.com
/// @github: https://github.com/cairuoyu/flutter_admin
/// @date: 2021/6/21
/// @version: 1.0
/// @description:

import 'package:cry/utils/http_util.dart';

class SubsystemApi {
  static listEnable() {
    return HttpUtil.post('/subsystem/listEnable');
  }

  static page(data) {
    return HttpUtil.post('/subsystem/page', data: data);
  }

  static saveOrUpdate(data) {
    return HttpUtil.post('/subsystem/saveOrUpdate', data: data);
  }

  static removeByIds(data) {
    return HttpUtil.post('/subsystem/removeByIds', data: data);
  }

  static loadById(String lib_path, String lib_name, int lib_type) {
    return HttpUtil.post('/api/module/load', data: {
      {'lib_path': lib_path},
      {"lib_name", lib_name},
      {"lib_type", lib_type}
    });
  }

  static unloadById(String lib_path, String lib_name, int lib_type) {
    return HttpUtil.post('/api/module/unload', data: {
      {"lib_name", lib_name}
    });
  }

  static enableById(String lib_path, String lib_name, int lib_type) {
    return HttpUtil.post('/api/module/enable', data: {
      {'lib_path': lib_path},
      {"lib_name", lib_name}
    });
  }

  static disableById(String lib_path, String lib_name, int lib_type) {
    return HttpUtil.post('/api/module/disable', data: {
      {"lib_name", lib_name}
    });
  }
}
