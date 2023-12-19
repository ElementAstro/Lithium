/*
 * ErrorHandle.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: Error Handle (404 or 500)

**************************************************/

#include "ErrorHandler.hpp"

#ifdef _cpp_fmt_lib
#include <format>
#else
#include <fmt/format.h>
#endif

ErrorHandler::ErrorHandler(const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &objectMapper)
    : m_objectMapper(objectMapper)
{
}

std::shared_ptr<ErrorHandler::OutgoingResponse>
ErrorHandler::handleError(const Status &status, const oatpp::String &message, const Headers &headers)
{
    auto error = StatusDto::createShared();
    error->status = "ERROR";
    error->code = status.code;
    error->message = message;

    if (status.code == 404)
    {
        std::stringstream htmlStream;
        htmlStream << "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'/><meta http-equiv='X-UA-Compatible'content='IE=edge'/><meta name='viewport'content='width=device-width, initial-scale=1.0'/><title>404</title><style>body{font-family:Segoe UI,sans-serif;background:#3973aa;color:#fefeff;height:100vh;margin:0}#page{display:table;height:100%;margin:0 auto;margin-top:-10px;width:70%;font-size:1.9vw}#container{display:table-cell;vertical-align:middle}h1,h2,h3,h4,h5{font-weight:normal;padding:0;margin:25px 0;margin-top:0}h1{font-size:6rem;margin-bottom:10px}h2{font-size:21px}h4{font-size:18px;line-height:1.5em}h5{line-height:22px;font-size:14px;font-weight:400}#details{display:flex;flex-flow:row;flex-wrap:nowrap;padding-top:10px}#qr{flex:0 1 auto}#image{background:white;padding:5px;line-height:0}#image img{width:9.8em;height:9.8em}#stopcode{padding-left:10px;flex:1 1 auto}@media(min-width:840px){#page{font-size:140%;width:800px}}a{color:#e1f0ff}</style></head><body><div id='page'><div id='container'><h1>:(</h1><h2>Lithium Server seemed to have crashed.We're sorry for the\n";
        htmlStream << "inconvenience. The server will automatically restart in a few minutes.\n";
        //htmlStream << fmt::format("We're just collecting some error info.</h2><h2><span id='percentage'>0</span>%complete</h2><div id='details'><div id='qr'><div id='image'><img src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAcIAAAHCCAYAAAB8GMlFAAAZmklEQVR4nO3c23IbOZAFwNbG/v8vax8civHasgSKaOJUVeajzMG9WcRIPG/v7+/vFwAM9T+nBwAAJymEAIymEAIwmkIIwGgKIQCjKYQAjKYQAjCaQgjAaAohAKMphACMphACMJpCCMBoCiEAoymEAIymEAIwmkIIwGgKIQCjKYQAjKYQAjCaQgjAaAohAKMphACMphACMJpCCMBoCiEAoymEAIymEAIwmkIIwGgKIQCjKYQAjKYQAjCaQgjAaAohAKMphACMphACMJpCCMBoCiEAoymEAIymEAIw2v+eHsAz3t7eTg/hVu/v76eHAGzi/SrX23vB0Xc/UH9K3aI/9+Ff49z9ut1293tqHtOkn6t/9d9dxfNeqhBOO1B/Stqqf+3Fn2Pc/brddvd7ah7TpJ+rr/qeotKZL/M7wumH6rpy1uCrcfz+b7tft9vufk/NY5r0c/WK9iuotAZlCiEA3KFEIaz0yeJu1gKyeUb/U2UtShRCALhLfCGs8onilU6vyVe/BP/933a/brfd/Z6axzTJ5+r0s5mowprEF0IyffZG8oqf7ba731PzmCb9XFFL/NcnKnyaOCF822Ak71efS3+/Kp0ssyptEzwswL94v3q91oUw7UB9+BjXhAP2iFNJNRJtnmsvfT/Sz8Gp/lZNeL9q+TvC9/f32EP1uyrjfIXPHrKkn+22u99T7aXvR/o5uK467wNVxvkT7QphxY2qOOadTiXVSLR5rr30/Ug/B9dV89mvOObvtCuEAPCIVoWw8ieVymMHHlf5ma889s+0KoQA8Kg2hbDDJ5QOc/iJU0k1Em2eay99P5LPQYdnvcMcPrQphNR2KqlGos1z7aXvR/o5IEObZJnwaSybNl/oZNrz22W+rb9Q/6hn/1Q6fbOBPrxf7aMQXvu+K/TRTuoBS08y2W3afFd1WZcqiTG7TXm/eqXRvyN8e3u75Quzd7X7jPQkk92mzXdVl3WpkBiz26T3q1cbWwhPPawnpCeZ7DZtvqu6rEuFxJjdJr1fnTC2EALAdQ0thK/85DP5UxbwPO9X9xtZCAHgw7hCeOITz+lPWelJJrtNm++qLuuSnBiz28T3qxPGFcKp0pNMdps231Vd1kViDDv5HuEgu98A0t9Qps13VZd1We23y75xHzdCAEZzIxxkdxLHqWSP9Hl0STJJX79p+8F93AiH2J3EcSrZI2nMnZNMktbKfnA3hXCA3Ukcp5I90ufRJckkff2m7Qf3UwgBGE0hBGA0hRCA0RTCAXYncZxK9kifR4ckk+vKX79p+8H9FMIhdidxnEr2SBpz5ySTpLWyH9zt7T38VKz+ddfqNE79tdju8YVvG4zk/eq59k5xIwRgNMkyg5xK2EhPlpnWb3pyS5dzSh1uhEOcSthIT5aZ1m96ckuXc0ot4wrhiU95pz9ZnkrYSE+WmdZvenJLl3O608T3qxPGFUIA+N3IQvjKTzwTP10B+3i/ut/IQggAH8YWwld88kn5dHUqYSM9WWZav+nJLV3O6R0mvV+dMLYQXtevjb9j8+9q9xmnEjbSk2Wm9Zue3NLlnN5h0vvVq41LltnRV8IYwrcNRvJ+dV9fd/KF+t+kbxbAB+9X+yiEg0xLFElPeDmly/i67Afnjf4d4STTEkXSE15O6TK+LvtBhjaFsMPhvmsO0xJF0hNeTukyvg77cbr/HTrM4UObQggAP9GqEFb+hFJ57MDjKj/zlcf+mVaFEAAe1a4QVvykcveYpyWKpCe8nNJlfF3247q8X6VoVwiv69dGVdisV45zWqJIesLLKV3G12U/rsv7VYI2yTJfSZtixzkBPZ/tjnP604gv1Hf9FAP04/3q9UYUQn7pktyiPe1VaI86Wv6OkL91SW7RnvYqtEct8YXQp7K/PbomXZJbtKe99Pa8X/2twprEF0IAuFOJQljhE8WrWAvI5hn9T5W1KFEIAeAuZQphlU8Wd/rpGnRJbtGe9iq098x/10mlNYj/Qv1npv01164tSv9zc+1pr1N7/2q3u4IlpWYh/ND9gBXeGuAP3q9ylS6EAPAsyTKDrP6vn/SEjS7jO/W/9tLPwbR+Oa/MH8vwnNXkjPSEjS7jO5WMkn4OpvVLBoVwgNXkjN2JHbt1Gd+pZJT0czCtX3IohACMphACMJpCCMBoCuEAq8kZdyRs7NRlfKeSUdLPwbR+yaEQDvHZA/3Mz07pMr7d89jd76l1ntYvGXyhHoDR3AgBGK1UskyXJI70ZI9V0xJAuvSbfq5Wpc8j/X0jff1eqcyNsEsSR3qyx6ppCSBd+k0/V6vS55H0HlHx/eXVShTCLkkc6ckeq6YlgHTpN/1crUqfR/r7Rvr6nVCiEALAXRRCAEZTCAEYrUQh7JLEkZ7ssWpaAkiXftPP1ar0eaS/b6Sv3wklCuF19UniSE/2WDUtAaRLv+nnalX6PJLeIyq+v7yaZBkARitzIwSAO7RMljmVrJCePNLldavSEzbS55s+vmn9djmnicrcCNOTFdKTR7r8bFV6wkb6fNPHN63fLuc0VYlCmJ6skJ480uV1q9ITNtLnmz6+af12OafJShRCALiLQgjAaAohAKOVKITpyQrpySNdXrcqPWEjfb7p45vWb5dzmqxEIbyu/GSF9OSRLj9blZ6wkT7f9PFN67fLOU0lWQaA0crcCAHgDqWSZbrokiQhmSdrfLvbS5/vtPFNm+8ruRG+WJckCck8WePb3V76fKeNb9p8X00hfKEuSRKSedbaeFZ64sluxvdcv7ul78dOCiEAoymEAIymEAIwmkL4Ql2SJCTzrLXxrPTEk92M77l+d0vfj50UwhfrkiQhmSdrfLvbS5/vtPFNm++rSZYBYDQ3QgBGK5Us0yVho0t76euXnohxah7p801vr0u/u6WP7ytlboRdEja6tJe+fumJGKfmkT7f9Pa69Ltb+vi+U6IQdknY6NJe+vqlJ2Kcmkf6fNPb69LvbunjW1GiEALAXRRCAEZTCAEYrUQh7JKw0aW99PVLT8Q4NY/0+aa316Xf3dLHt6JEIbyuPgkbXdpLX7/0RIxT80ifb3p7XfrdLX1835EsA8BoZW6EAHCHlsky05IzuvS7Kj2RRYLKa/rtcv5297u7vfT92KHMjTA9gaFL4kSXdU7/2aou+5u+zqvS1293e+n7sUuJQpiewNAlcaLLOqe/blWX/U1f51Xp67e7vfT92KlEIQSAuyiEAIymEAIwWolCmJ7A0CVxoss6p79uVZf9TV/nVenrt7u99P3YqUQhvK78BIYuiRNd1jn9Z6u67G/6Oq9KX7/d7aXvxy6SZQAYrcyNEADuUCpZZlXlhIOf6JIQkT6Pae2l99uF9Tuv3Y2wesLBo7okRKTPY1p76f12Yf0ytCqEHRIOHtElISJ9HtPaS++3C+uXo1UhBIBHKYQAjKYQAjBaq0LYIeHgEV0SItLnMa299H67sH45WhXC66qfcPCoLgkR6fOY1l56v11YvwySZQAYrd2NEAAeUSpZpkuiyO5+0xNjVnVZ5939dmnvlC7nKv2cVj4vZW6EXRJFdvebnhizqss67+63S3undDlX6ee0+nkpUQi7JIrs7jc9MWZVl3Xe3W+X9k7pcq7Sz2mH81KiEALAXRRCAEZTCAEYrUQh7JIosrvf9MSYVV3WeXe/Xdo7pcu5Sj+nHc5LiUJ4XX0SRXb3m54Ys6rLOu/ut0t7p3Q5V+nntPp5kSwDwGhlboQAcIdSyTK7VU5C4HtdEjG6zGM3iTE155to7I2wehICX+uSiNFlHrtJjKk531QjC2GHJAT+rUsiRpd57CYxZm1Mz0of304jCyEAfFAIARhNIQRgtJGFsEMSAv/WJRGjyzx2kxizNqZnpY9vp5GF8LrqJyHwtS6JGF3msZvEmJrzTSVZBoDRxt4IAeC6hifLrNqdmNAlcWK3LvNYlZ5Q0uWcnhrftHWp/Py6EX5jd2JCl8SJ3brMY1VSGknSz3Y7Nb5p61L9+VUIv7A7MaFL4sRuXeaxKj2hpMs5PTW+aevS4flVCAEYTSEEYDSFEIDRFMIv7E5M6JI4sVuXeaxKTyjpck5PjW/aunR4fhXCb+xOTOiSOLFbl3msSkojSfrZbqfGN21dqj+/kmUAGM2NEIDRWibLVE44qKRLYse0hI0u4+ty/nY7Nd/K2t0IqyccVNElsWNawkaX8XU5f7udmm91rQphh4SDCrokdkxL2Ogyvi7nb7dT8+2gVSEEgEcphACMphACMFqrQtgh4aCCLokd0xI2uoyvy/nb7dR8O2hVCK+rfsJBFV0SO6YlbHQZX5fzt9up+VYnWQaA0drdCAHgEaWSZdITDtITLNKTOHZzXrL2N73faee0y/rtUOZGmJ5wkJ5gkZ7EsZvzkrW/6f1OO6dd1m+XEoUwPeEgPcEiPYljN+fl83/T73Ov2y293/T126lEIQSAuyiEAIymEAIwWolCmJ5wkJ5gkZ7EsZvz8vm/6fe51+2W3m/6+u1UohBeV37CQXqCRXoSx27OS9b+pvc77Zx2Wb9dJMsAMFqZGyEA3KFUssyqLoksu6XPt8v4TrW3u9/0c59+DtLXpUu/O7S7ESalryR9mTR9vl3Gd6q93f2mn/v0c5C+Ll363aVVIeySyLJb+ny7jO9Ue7v7TT/36ecgfV269LtTq0IIAI9SCAEYTSEEYLRWhbBLIstu6fPtMr5T7e3uN/3cp5+D9HXp0u9OrQrhdWWlryQdgvT5dhnfqfZ295t+7tPPQfq6dOl3F8kyAIzW7kYIAI9omSyzqksyxbQkifQEi/TxreqS0JT+/Kb3O8HYG2GXZIppSRLpCRbp41uVlMaUlOCTPo8u5+/VRhbCLskU05Ik0hMs0se3qktCU/rzm97vJCMLIQB8UAgBGE0hBGC0kYWwSzLFtCSJ9ASL9PGt6pLQlP78pvc7ychCeF19kimmJUmkJ1ikj29VUhpTUoJP+jy6nL9XkywDwGhjb4QAcF3FkmXSky4krXyuS8JGl3PVZXy7pT/nq7rsxyuVuREmpVokJTqkJ0l0SdhIOkMV57Eq/TnyvGXNd5cShTA96ULSyuNjqJSw0eVcdRnfbunP+aou+3FCiUIIAHdRCAEYTSEEYLQShTA96ULSyuNjqJSw0eVcdRnfbunP+aou+3FCiUJ4XVmpFkmJDulJEl0SNpLOUMV5rEp/jjxvWfPdRbIMAKOVuRECwB1GJ8uc6rfL61alzyN9/Valj29V+r51eT5WdTlXXylzI0xPupj2s1VJY0762W7p41uVtEedn49VXc7Vd0oUwvSki2mvW5U+j/T1W5U+vlXp+9bl+VjV5VytKFEIAeAuCiEAoymEAIxWohCmJ11Me92q9Hmkr9+q9PGtSt+3Ls/Hqi7nakWJQnhd+UkX0362KmnMST/bLX18q5L2qPPzsarLufqOZBkARitzIwSAO0iW2djvqfbSdVm/9CST9ISSVdOSUbqMr7IyN8L0ZIVT7aXrsn5JqSVJP9ttWjJKl/FVV6IQpicrnGovXZf1S08ySU8oWTUtGaXL+DooUQgB4C4KIQCjKYQAjFaiEKYnK5xqL12X9UtPMklPKFk1LRmly/g6KFEIrys/WeFUe+m6rF9SaknSz3ablozSZXzVSZYBYLQyN0IAuEOpZJlV0xI20sfXRZd17rJvXZKmTrEu/2l3I5yWsJE+vi66rHOXfeuSNHWKdfn/WhXCaQkb6ePross6d9m3LklTp1iXv7UqhADwKIUQgNEUQgBGa1UIpyVspI+viy7r3GXfuiRNnWJd/taqEF7XvISN9PF10WWdu+xbl6SpU6zL/ydZBoDR2t0IAeARpZJl0pMLuiRdnGpPv1n9po9vWr/pz3llZW6E6ckFXZIuTrWn36x+08c3rd/057y6EoUwPbmgS9LFqfb0m9Vv+vim9Zv+nHdQohACwF0UQgBGUwgBGK1EIUxPLuiSdHGqPf1m9Zs+vmn9pj/nHZQohNeVn1zQJeniVHv6zeo3fXzT+k1/zquTLAPAaGVuhABwB8ky+v1xv+lJHPqtKX1dupyX9OSbVypzIzyVhKDfrKQL/fb+0nP6unQ5L+nJN69WohCeSkLQ7+f/lp7Eod+a0tely3lJT745oUQhBIC7KIQAjKYQAjBaiUJ4KglBv5//W3oSh35rSl+XLuclPfnmhBKF8LrOJSHoNyvpQr913lx+In1dupyX9OSbV5MsA8BoZW6EAHCH0ckyXRIYurSXnhRyinPau70u+1FZmRvhtKSGVV3aS08KOSXpTFY8V+ntddmP6koUwmlJDau6tJeeFHKKc9q7vS770UGJQggAd1EIARhNIQRgtBKFcFpSw6ou7aUnhZzinPZur8t+dFCiEF7XvKSGVV3aS08KOSXpTFY8V+ntddmP6iTLADBamRshANxBssyB9tKTJNLnsSp9nU+1d6rfLu11ed2qCQk0ZW6E6ckKXZIk0uexKn2dT7V3qt8u7XX52aopCTQlCmF6skKXJIn0eaxKX+dT7Z3qt0t7XV63alICTYlCCAB3UQgBGE0hBGC0EoUwPVmhS5JE+jxWpa/zqfZO9dulvS6vWzUpgaZEIbyu/GSFLkkS6fNYlb7Op9o71W+X9rr8bNWUBBrJMgCMVuZGCAB3KJUsw3OmJcF0ScRITxSZts6npJ+DytwIh5iWBNMlESM9UWTaOp+Sfg6qUwgHmJYE0yURIz1RZNo6n5J+DjpQCAEYTSEEYDSFEIDRFMIBpiXBdEnESE8UmbbOp6Sfgw4UwiGmJcF0ScRITxSZts6npJ+D6iTLADBa6S/Ud/sT3j/5jAJwv5KFsHsB/PAxz1cXxPRkilMJNOnrspv5Zs23y3OUqNT/Gp1SAP/lFVv1rzX+ad+n2jv1ulW729vNfH9JmW+X5yhVmT+WmV4Er+v+NUhPpjiVQJO+LruZ79q/dW9vUsJQmUIIAHcoUQirfKp4BWsBsFeJQggAd4kvhG5Af7trTdKTKU4l0KSvy27mu/Zv3dublDAUXwh5rfRkilMJNOnrspv5Zs23y3OUKv7rE26EnwvfNoAySn6h/lFpRUNxB8jRuhCmFcAPH+N6dUFMT6bYrUtyRuXEjjulJ7fsJgHpPi1/R/j+/l5is145zs+K7jOFeLW93f2u2j2+9HlMc+o8n3LqnKavyy7tCmGFAvinu8ecnkyxW5fkjA6JHXdIT27ZTQLS/doVQgB4RKtCWPE2+KHy2AEqa1UIAeBRbQphhxvVXXNIT6bYrUtyRofEjjukJ7fsJgHpfm0KIV9LT6bYrUtyRvXEjrukJ7fsJgHpXm2SZcKnsWzafAFOa/2F+kc9+yfBihNAPQrhte87MR/tpBbEU0kr6cktp6Qn/XTZ3/T5dkmMqfz8jv4d4dvb2y1fDL2r3WecSlpJT245JT3pp8v+Js0taf3S23u1sYXw1EN4wqmklfTkllPSk3667G/6fLskxnR4fscWQgC4rqGF8JWfUqp8IgKYamQhBIAP4wrhiRva6VvhqaSV9OSWU9KTfrrsb/p8uyTGdHh+xxXCqU4lraQnt5ySnvTTZX+T5pa0funtvdq4ZJlTt7Pd4wvfNoAy3AgBGE2yzCDpSSa7+93dXuXkjN9Nm0eX8zLtnL6SG+EQ6Ukmu/vd3V715IwP0+bR5bxMO6evphAOkJ5ksrvf3e11SM64rnnz6HJepp3TExRCAEZTCAEYTSEEYDSFcID0JJPd/e5ur0NyxnXNm0eX8zLtnJ6gEA6RnmSyu9/d7VVPzvgwbR5dzsu0c/pqkmVeRLIMQCY3QgBGkywzyKnEifRkmS52J4+cet2q9PFNU3n93AiHOJU4kZ4s08Xu5JFTP1uVPr5pqq/fuEJ44lPK6U9GpxIn0pNlutidPHLqdavSxzdNh/UbVwgB4HcjC+Erb2inb4MAfG1kIQSAD2ML4Stuaim3wVOJE+nJMl3sTh459bpV6eObpsP6jS2E1/Vrk+7YqLvafcapxIn0ZJkudiePnPrZqvTxTVN9/cYly+zoK2EM4dsGUIYv1P9GcQGYRyHkx6YlxqSPb1WXfUvfj/Qknd3S9+Mro39HyM9NS4xJH9+qLvuWvh/pSTq7pe/Hd9oUwkqL/i9V5jAtMSZ9fKu67Fv6fqQn6eyWvh8r2hRCAPiJVoWwyqePz1QeO0BlrQohADyqXSGseLOqNuZpiTHp41vVZd/S9yM9SWe39P1Y0a4QXtevwlKhuFQZ52emJcakj29Vl31L34/0JJ3d0vfjO22SZb6SNsWOcwKoasQX6qveugC434hCyC+7kym6vG63Lv2m79upc5De727pz9sOLX9HyN92J1N0+dluXfpN2qOkc5De727pz9su8YWw0qeKV3l0TXYnU3R53W5d+k3ft1PnIL3f3dKft53iCyEA3KlEIXQr/I+1ANirRCEEgLuUKYRuQj9fg93JFF1et1uXftP37dQ5SO93t/Tnbaf4L9R/psovYHfZtUXpf76e/mfzq7r0m75v6V9j8PWJOqWlZCH80L0gFt4agDJKf6FeoQDgWWV+RwgAd1AIARhNIQRgNIUQgNEUQgBGUwgBGE0hBGA0hRCA0RRCAEZTCAEYTSEEYDSFEIDRFEIARlMIARhNIQRgNIUQgNEUQgBGUwgBGE0hBGA0hRCA0RRCAEZTCAEYTSEEYDSFEIDRFEIARlMIARhNIQRgNIUQgNEUQgBGUwgBGE0hBGA0hRCA0RRCAEZTCAEYTSEEYDSFEIDRFEIARlMIARhNIQRgtP8Dv2cZZnuA8MkAAAAASUVORK5CYII='alt='QR Code'/></div></div><div id='stopcode'><h4>For more information about this issue and possible fixes,visit<br/><a href='https://github.com/ElementAstro/Lithium/issues'>https:</a></h4><h5>If you call a support person,give them this info:<br/>Stop Code:{}Message:{}</h5></div></div></div></div><script>var percentageElement=document.getElementById('percentage');var percentage=0;function process(){percentage+=parseInt(Math.random()*10);if(percentage>100){percentage=100}percentageElement.innerText=percentage;processInterval()}function processInterval(){setTimeout(process,Math.random()*(1000-500)+500)}processInterval();</script></body></html>", error->code, error->message);
        auto response = ResponseFactory::createResponse(Status::CODE_404, htmlStream.str());
        return response;
    }
    else
    {
        auto response = ResponseFactory::createResponse(status, error, m_objectMapper);

        for (const auto &pair : headers.getAll())
        {
            response->putHeader(pair.first.toString(), pair.second.toString());
        }
        return response;
    }
}