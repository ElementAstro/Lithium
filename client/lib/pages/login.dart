/// @author: cairuoyu
/// @homepage: http://cairuoyu.com
/// @github: https://github.com/cairuoyu/flutter_admin
/// @date: 2021/6/21
/// @version: 1.0
/// @description: 登录

import 'package:flutter/material.dart';
import 'package:cry/cry.dart';
import 'package:flutter_admin/constants/constant.dart';
import 'package:cry/model/response_body_api.dart';
import 'package:flutter_admin/pages/common/lang_switch.dart';
import 'package:flutter_admin/utils/store_util.dart';
import 'package:flutter_admin/api/user_api.dart';
import 'package:flutter_admin/models/user.dart';
import '../generated/l10n.dart';

class Login extends StatefulWidget {
  @override
  _LoginState createState() => _LoginState();
}

class _LoginState extends State<Login> {
  final GlobalKey<FormState> formKey = GlobalKey<FormState>();
  User user = new User();
  String error = "";
  FocusNode focusNodeUserName = FocusNode();
  FocusNode focusNodePassword = FocusNode();
  FocusNode focusNodeServerHost = FocusNode();
  FocusNode focusNodeServerPort = FocusNode();
  FocusNode focusNodeIsSSLEnabled = FocusNode();
  FocusNode focusNodeTestConnection = FocusNode();
  bool isSSLEnabled = false;

  @override
  void initState() {
    super.initState();
    focusNodeUserName.requestFocus();
  }

  @override
  Widget build(BuildContext context) {
    return Theme(
      data: ThemeData(),
      child: Scaffold(
        body: _buildPageContent(),
      ),
    );
  }

  Widget _buildPageContent() {
    var appName = Text(
      S.of(context).appName,
      style: TextStyle(fontSize: 16, color: Color.fromARGB(255, 6, 157, 251)),
      textScaler: TextScaler.linear(3.2),
    );
    return Container(
      child: ListView(
        children: <Widget>[
          Column(
            crossAxisAlignment: CrossAxisAlignment.end,
            mainAxisAlignment: MainAxisAlignment.end,
            children: [LangSwitch()],
          ),
          Center(child: appName),
          _buildLoginForm(),
          // SizedBox(height: 20.0),
        ],
      ),
    );
  }

  Container _buildLoginForm() {
    return Container(
      child: Stack(
        children: <Widget>[
          Center(
            child: Container(
              margin: EdgeInsets.only(top: 40),
              padding: EdgeInsets.all(10.0),
              decoration: BoxDecoration(
                borderRadius: BorderRadius.all(Radius.circular(40.0)),
                color: Colors.white,
              ),
              child: Form(
                key: formKey,
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: <Widget>[
                    Container(
                      padding: EdgeInsets.fromLTRB(20, 0, 20, 20),
                      child: TextFormField(
                        focusNode: focusNodeServerHost,
                        obscureText: true,
                        initialValue: user.serverHost,
                        style: TextStyle(color: Colors.black),
                        decoration: InputDecoration(
                          border: OutlineInputBorder(),
                          labelText: S.of(context).serverHost,
                          icon: Icon(
                            Icons.cloud_queue,
                            color: Colors.blue,
                          ),
                        ),
                        onSaved: (v) {
                          user.serverHost = v;
                        },
                        validator: (v) {
                          return v!.isEmpty
                              ? S.of(context).serverHostRequired
                              : null;
                        },
                        onFieldSubmitted: (v) {
                          _login();
                        },
                      ),
                    ),
                    Container(
                      padding: EdgeInsets.fromLTRB(20, 0, 20, 20),
                      child: TextFormField(
                        focusNode: focusNodeServerPort,
                        obscureText: true,
                        initialValue: user.serverPort,
                        style: TextStyle(color: Colors.black),
                        decoration: InputDecoration(
                          border: OutlineInputBorder(),
                          labelText: S.of(context).serverPort,
                          icon: Icon(
                            Icons.data_array,
                            color: Colors.blue,
                          ),
                        ),
                        onSaved: (v) {
                          user.serverPort = v;
                        },
                        validator: (v) {
                          return v!.isEmpty
                              ? S.of(context).serverPortRequired
                              : null;
                        },
                        onFieldSubmitted: (v) {
                          _login();
                        },
                      ),
                    ),
                    Container(
                      padding: EdgeInsets.fromLTRB(20, 0, 20, 20),
                      child: TextFormField(
                        focusNode: focusNodeUserName,
                        initialValue: user.userName,
                        style: TextStyle(color: Colors.black),
                        decoration: InputDecoration(
                          border: OutlineInputBorder(),
                          labelText: S.of(context).username,
                          icon: Icon(
                            Icons.people,
                            color: Colors.blue,
                          ),
                        ),
                        onSaved: (v) {
                          user.userName = v;
                        },
                        validator: (v) {
                          return v!.isEmpty
                              ? S.of(context).usernameRequired
                              : null;
                        },
                        onFieldSubmitted: (v) {
                          focusNodePassword.requestFocus();
                        },
                      ),
                    ),
                    Container(
                      padding: EdgeInsets.fromLTRB(20, 0, 20, 20),
                      child: TextFormField(
                        focusNode: focusNodePassword,
                        obscureText: true,
                        initialValue: user.password,
                        style: TextStyle(color: Colors.black),
                        decoration: InputDecoration(
                          border: OutlineInputBorder(),
                          labelText: S.of(context).password,
                          icon: Icon(
                            Icons.lock,
                            color: Colors.blue,
                          ),
                        ),
                        onSaved: (v) {
                          user.password = v;
                        },
                        validator: (v) {
                          return v!.isEmpty
                              ? S.of(context).passwordRequired
                              : null;
                        },
                        onFieldSubmitted: (v) {
                          _login();
                        },
                      ),
                    ),
                    Container(
                      child: CheckboxListTile(
                        value: user.isSSLEnabled,
                        onChanged: (value) {
                          //第二个复选框列表 item
                          setState(() {
                            user.isSSLEnabled = value ?? false; //更新复选框的状态
                          });
                        },
                        activeColor: Colors.blue, //选中的背景颜色，如果 selected 为 true ，则 title，subtitle，secondary 也会变
                        checkColor: Colors.black, //选中复选框里面符号的颜色
                        title: Text(S.of(context).isSSLModeEnabled), //展示标题
                        subtitle: Text(S.of(context).isSSLModeEnabledDesc), //展示副标题
                      ),
                    ),
                    Container(
                      alignment: Alignment.bottomCenter,
                      child: SizedBox(
                        width: 420,
                        child: ElevatedButton(
                          onPressed: _login,
                          style: ButtonStyle(
                            shape: MaterialStateProperty.all(
                                RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(40.0))),
                          ),
                          child: Text(S.of(context).login,
                              style: TextStyle(
                                  color: Color.fromARGB(179, 5, 121, 237),
                                  fontSize: 20)),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }

  _register() {
    Cry.pushNamed('/register');
  }

  _login() async {
    var form = formKey.currentState!;
    if (!form.validate()) {
      return;
    }
    form.save();

    ResponseBodyApi responseBodyApi = await UserApi.login(user.toMap());
    if (!responseBodyApi.success!) {
      focusNodePassword.requestFocus();
      return;
    }
    _loginSuccess(responseBodyApi);
  }

  _loginSuccess(ResponseBodyApi responseBodyApi) async {
    StoreUtil.write(
        Constant.KEY_TOKEN, responseBodyApi.data[Constant.KEY_TOKEN]);
    StoreUtil.write(Constant.KEY_CURRENT_USER_INFO,
        responseBodyApi.data[Constant.KEY_CURRENT_USER_INFO]);
    await StoreUtil.loadDict();
    await StoreUtil.loadSubsystem();
    await StoreUtil.loadMenuData();
    await StoreUtil.loadDefaultTabs();
    StoreUtil.init();

    Cry.pushNamed('/');
  }
}
