# iOS Application File

最近一款在VirusTotal的应用程序引起了我们的注意

```
$ openssl dgst -sha256 com.mailtime.MailTimePro-clutch2.ipa
SHA256(com.mailtime.MailTimePro-clutch2.ipa)= 332cf0a45170d6787dcbefb086f5a5f0f6e920d485e377fe37e900a01c128c8e
```
从文件名可以看出这是一款使用了Clutch的破解软件。对ipa文件进行解压（实际是zip文件）
```
$ ditto -xk com.mailtime.MailTimePro-clutch2.ipa com.mailtime.MailTimePro-clutch2

$ cd "com.mailtime.MailTimePro-clutch2/Payload/MailTime Pro.app/"

$ find . -type f -exec file {} \; | grep "Mach-O"
(...)
./jailbreak: Mach-O universal binary with 2 architectures
./jailbreak (for architecture armv7): Mach-O dynamically linked shared library arm
./jailbreak (for architecture arm64): Mach-O 64-bit dynamically linked shared library
./MailTime Pro: Mach-O universal binary with 2 architectures
./MailTime Pro (for architecture armv7): Mach-O executable arm
./MailTime Pro (for architecture arm64): Mach-O 64-bit executable
```
jailbreak这个库看起来很可疑
```
$ codesign -dvv "MailTime Pro" 2>&1 | grep Authority
Authority=iPhone Developer: nguyen tat hung (T99T9WYY54)
Authority=Apple Worldwide Developer Relations Certification Authority
Authority=Apple Root CA

$ codesign -dvv jailbreak 2>&1 | grep Authority
Authority=iPhone Developer: nguyen tat hung (T99T9WYY54)
Authority=Apple Worldwide Developer Relations Certification Authority
Authority=Apple Root CA
```
nguyen tat hung 虽然是一个知名开发商，但不是MailTime Pro的开发商。
进一步，我们发现jailbreak动态注入到了二进制程序中。
```
$ otool -arch arm64 -L "MailTime Pro"
(...)
    @executable_path/jailbreak (compatibility version 0.0.0, current version 0.0.0)
```
大多数的库，在导入表中包含了他们的安装路径和真实名称
```
$ otool -arch arm64 -L jailbreak
jailbreak:
    /usr/local/lib/libguiinject.dylib (compatibility version 1.0.0, current version 1.0.0)
(...)
```
通过assert()宏，泄露了项目的关联信息
```
$ strings -arch arm64 jailbreak | grep -i guiinject
/Users/gtt/Documents/workspaceIOS/guiinject/guiinject/MBProgressHUD.m
/Users/gtt/Documents/workspaceIOS/guiinject/guiinject/SSZipArchive.m
guiinject
```

# Inject Library
通过符号表，快速的找到知名SDK和CoCoa Pod。

#### Advertisement SDKs
    Carrot
    Facebook Audience Network
    Google AdMob
    StartApp
#### Cocoa Pods
    FileMD5Hash
    MBProgressHUD
    SSZipArchive

一个越狱软件包括如此多的广告是不常见的，这里有剩余的类[头文件](http://pastebin.com/Zjw1eYk9)

**-[Config getConfig]方法从程序目录加载wrap.json文件**
```
{
  "udid": "jailbreak",
  "wait_loop": "3",
  "is_jb": "1",
  "package_name": "com.mailtime.MailTimePro"
}
```
经过几次循环等待，注入的代码会联系一个不安全的远程主机，部分的服务可以工作。

比如，coreapi服务似乎不能工作。

**http://wrapper.laughworld.net/coreapi/active_device.php?pk=IPANAME&is_jb=1&udid=REDACTED&signature=MD5**:
```
{
  "return": -2,
  "message": "DB operator fail!"
}
```
**http://wrapper.laughworld.net/coreapi/get_list_message.php?pk=IPANAME&is_jb=1&udid=REDACTED&libver=20160818&app_pk=IPANAME_AGAIN&app_ver=1.2.3&signature=MD5**:
```
{
  "return": 0,
  "messages": []
}
```
当api服务启动后：

**http://wrapper.laughworld.net/api/com.mailtime.MailTimePro_ads.json**:
```
{
  "advertising_list": [
    {
      "id": 1,
      "act_type": "0",
      "b": "<body><iframe style='border:none;padding-left:0px;padding-top:0px;' src='http://bypassfirewall.net'><p>http://bypassfirewall.net/</p></iframe></body>",
      "dp_type": "1",
      "url": "http://bypassfirewall.net",
      "hide": 1,
      "random_show": "5",
      "adsnet_name": "admob",
      "adsnet_id": "ca-app-pub-3816529472258726/8039356495"
    }
  ]
}
```
最有趣的是update请求。

**-[API getUpdate:withSelector:]方法会请求http://wrapper.laughworld.net/api/com.mailtime.MailTimePro_update.json**:
```
{
  "show_ads": "YES",
  "show_message": "YES",
  "update_message_not": "",
  "update_link": "http://google.com",
  "linkfw": "http://wrapper.laughworld.net/lib/DailyUploadDownloadLib.framework.zip",
  "namefw": "DailyUploadDownloadLib.framework",
  "md5fw": "f6a51b479516f11ce503ae06f9ffff0f",
  "script_zip": "http://wrapper.laughworld.net/lib/filehost.scr.zip",
  "script_file": "filehost.scr",
  "md5_script": "a9ef52dc75ecbcfce9447237f5154417"
}
```
script_zip,script_file和md5_script没有被执行，script_zip的URL指向一个加密的ZIP,并且md5_script是无效的(最后一个字节错误)。

linkfw指向一个有效的Zip，一旦下载并解压， -[guiinject _loadPluginAtLocation:]将加载框架并发送一个run消息到principalClass。md5fw用于自我更新。

到目前为止，我们没有看到任何广告，他们可能隐藏在视图中。

# Downloaded Framework
```
$ openssl dgst -sha256 DailyUploadDownloadLib
SHA256(DailyUploadDownloadLib)= 00ca48ebeda3d93ccf1b8b405fcf4c2062424bbc99425e27f0b65c7ee238780e

$ file DailyUploadDownloadLib
DailyUploadDownloadLib: Mach-O universal binary with 2 architectures
DailyUploadDownloadLib (for architecture armv7): Mach-O dynamically linked shared library arm
DailyUploadDownloadLib (for architecture arm64): Mach-O 64-bit dynamically linked shared library
```
开发者的标识发生了改变。
```
$ codesign -dvv DailyUploadDownloadLib 2>&1 | grep Authority
Authority=iPhone Developer: Pham Hiep (8DYXPR6ZBP)
Authority=Apple Worldwide Developer Relations Certification Authority
Authority=Apple Root CA
```
框架的header中有开发者和组织。
```
$ cat Headers/DailyUploadDownloadLib.h
//
// DailyUploadDownloadLib.h
// DailyUploadDownloadLib
//
// Created by GTT Media Hanoi on 9/29/16.
// Copyright © 2016 T&B. All rights reserved.
(...)
```
这是有Xcode自动生成的，经itviec工作网查询， GTT Media 和 T&B 是外包公司。

一旦加载完成，框架通过lib服务请求托管在DailyUploads和FileFactory站点上的文件列表。

**http://wrapper.laughworld.net/lib/DailyUploadDownloadModule.conf**:
```
{
  "list": [
    "https://dailyuploads.net/hdo3rn24n5tg",
    "https://dailyuploads.net/udzjx12rvu0z",
    "https://dailyuploads.net/8buwsi2hk9x7",
    "https://dailyuploads.net/vgnqrv66hp4l",
    "https://dailyuploads.net/mla2ofh3c0z8",
    "https://dailyuploads.net/ud2hlgpw9dto",
    "https://dailyuploads.net/030p4rn9ll6a",
    "https://dailyuploads.net/rsjbhbc6zi0b",
    "https://dailyuploads.net/wzrqhpqa7x7w",
    "https://dailyuploads.net/dqcl45a61amy",
    "https://dailyuploads.net/rhkbzrodo6ou",
    "https://dailyuploads.net/cqrakbup91s4",
    "https://dailyuploads.net/yxskttjfo4h8",
    "https://dailyuploads.net/m6p6maeijff1",
    "https://dailyuploads.net/f15g1prokvks"
  ]
}
```
**http://wrapper.laughworld.net/lib/FileFactoryDownloadModule.conf**:
```
{
  "list": [
    "http://filefactory.com/file/ebdz39d8dex/myfile42.encrypt",
    (...)
    "http://filefactory.com/file/1ycfrbml51ox/myfile7.encrypt",
    "http://filefactory.com/file/1k97tfd8ibu5/kdiff3-0.9.98-MacOSX-64Bit.dmg",
    "http://filefactory.com/file/5difpf82yog1/Newsgroup_collection.zip",
    "http://filefactory.com/file/6mlwn7iv1mv7/docword.enron.txt.gz",
    "http://filefactory.com/file/52sg5aurgkrz/Tiny_Wings__Andreas_Illiger___v2.1_os43_-Nitrox.rc330_84.ipa",
    "http://filefactory.com/file/6x9iujr8u6a5/php-5.6.14.tar.bz2",
    "http://filefactory.com/file/q29tth3j859/iBackupBot-Setup.dmg",
    "http://filefactory.com/file/6bcmlwfuw7wl/pokegoppsl.zip",
    "http://filefactory.com/file/4qqx4s5l36hn/iPhoneConfigUtility.dmg",
    "http://filefactory.com/file/5vqawo60iyb9/googlemobileadssdkios.zip",
    "http://filefactory.com/file/560caqad3k9h/mallet-2.0.8RC3.tar.gz",
    "http://filefactory.com/file/5ovqpwwp0w7h/609704981.ipa",
    "http://filefactory.com/file/1lpoyv8v2y73/Multiplayer_for_Minecraft_PE__v2.0_v2.012_Univ_os80_-Locophone-ICPDA.rc333_91.ipa",
    "http://filefactory.com/file/2abv5ufb9gav/MtProtoKit-master.zip",
    "http://filefactory.com/file/5t8e4px5fod1/577499909.ipa",
    "http://filefactory.com/file/4zy55s6qayrh/intel_rst_7_mb_8.1.zip",
    "http://filefactory.com/file/12toqn6khwd3/MEAD-3.12.tar.gz"
  ]
}
```
框架也会使用DynDNS定期检查iOS设备的外部IP。

每当框架加载完毕或者IP变更，都会从DailyUploads和FileFactory下载一个随机文件。

dailyuploads也指向其他iOS应用，这些应用都是被nguyen tat hung签名并注入的。

```
$ yara -r iOS.GuiInject.yara DailyUploads
ipa_jb DailyUploads/com.infinear.call-clutch2.ipa
ipa_jb DailyUploads/com.axidep.polyglotvoicereader-clutch2.ipa
ipa_jb DailyUploads/com.contrast.mileagelog-clutch2.ipa
ipa_jb DailyUploads/com.kymatica.AUFX-Space-clutch2.ipa
ipa_jb DailyUploads/co.qapps.calcpro-clutch2.ipa
ipa_jb DailyUploads/com.pixiapps.ecoutemobile-clutch2.ipa
ipa_jb DailyUploads/com.jhnd.blender-clutch2.ipa
ipa_jb DailyUploads/com.jackadam.darksky-rc.ipa
ipa_jb DailyUploads/com.markelsoft.Text2Speech-clutch2.ipa
ipa_jb DailyUploads/com.giacomoballi.FindTower-clutch2.ipa
ipa_jb DailyUploads/com.venderbase.dd-wrt-clutch2.ipa
ipa_jb DailyUploads/com.vincenzoaccardi.itracking-clutch2.ipa
ipa_jb DailyUploads/com.realvnc.VNCViewer-clutch2.ipa
ipa_jb DailyUploads/com.yacreader.yacreader-clutch2.ipa
ipa_jb DailyUploads/com.plumamazing.iWatermark-clutch2.ipa
```
# Abusing the Injected Adware Library
一个动态库使用不安全的协议来连接远程主机并执行代码。

通过拦截和修改update的response，我们成功加载了自己的代码。

![](https://sentinelone.staging.wpengine.com/wp-content/uploads/2016/10/abuse-169x300.jpg)

这是使用中间人来执行任意代码。我们应该感谢开发者在App中允许使用NSAllowsArbitraryLoads。

# Hundreds of Samples
通过VirusTotal Retrohunt和一些[yara规则](http://pastebin.com/2W0tyUAF)，我们找到了数百个样本。

# Conclusion
越狱的动机是研究，而不是盗版。生产设备必须远离越狱和盗版。

真正的破解者不以赚钱为目的。大多数开发商都提供自己的免费版产品和广告。那些广告收入应该归属原作者，而不是自私的破解者。

而当我们在这方面，由于最新的iOS设备的价格，人们可以获得数百个程序，并改善他们当前的设备体验。他们甚至不必为苹果感到内疚，因为母公司削减了App Store的销售额。

结论就是：破解者用隐藏的广告和假的下载来赚钱。用户浪费的流量，或者潜在滥用注入的广告软件库作为读者的练习。
