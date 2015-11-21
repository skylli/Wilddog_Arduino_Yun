### 简介
`Wilddog\_Arduino\_Sdk` 是一个arduino库，通过该库你的arduino Yun可以非常轻松的访问和更新云端的数据。目前`Wilddog\_Arduino\_Sd`k 仅支持`Arduino Yun`，用户通过Api向云端发送的请求先是通过`ar9331`转发到云端的，因为我们的库有两部分，如下：

	.
	├── ArduinoLibrary
	├── ArduinoYun_ar9331_Bin

	`ArduinoLibrary`为arduino库，包含用户调用接口的实现.
	`ArduinoYun_ar9331_Bin`为ar9331上的可执行文件，arduino和云端中转的实现.

### 使用步骤

####第一步 创建账号和应用

首先[**注册**](https://www.wilddog.com/account/signup)并登录Wilddog账号，进入控制面板。在控制面板中，添加一个新的应用。

你会获得一个独一无二的应用URL https://<appId>.wilddogio.com/，在同步和存取数据的时候，你的数据将保存在这个URL下。

####第二步 安装
	
#####Ar9331
######1、通过ssh登录Ar93313，具体操作步骤请移步`https://www.arduino.cc/en/Guide/ArduinoYun`。
######2、点击`ArduinoYun_ar9331_Bin`，并传输到Ar9331里，推荐使用SecureCRT传输文件，如下：
	root@Arduino:~# rz	（选择ArduinoYun_ar9331_Bin）
	root@Arduino:~# ls
	ArduinoYun_ar9331_Bin.zip
	root@Arduino:~# tar zxvf  ArduinoYun_ar9331_Bin.tar.gz
######3、把`ArduinoYun_ar9331_Bin`解压并添加系统`PATH`:
	root@Arduino:~# tar zxvf ArduinoYun_ar9331_Bin.tar.gz
	root@Arduino:~# cd ArduinoYun_ar9331_Bin
	root@Arduino:~/ArduinoYun_ar9331_Bin# ls
	install.sh        uninstall.sh      wilddog_daemon    wilddog_transfer
	root@Arduino:~/ArduinoYun_ar9331_Bin# sh install.sh
	root@Arduino:~/ArduinoYun_ar9331_Bin# ls /user/bin/wilddog*
	/usr/bin/wilddog_daemon    /usr/bin/wilddog_transfer  (有这两个文件代表安装成功)
		
######4、卸载`ArduinoYun_ar9331_Bin`:
	root@Arduino:~/ArduinoYun_ar9331_Bin# sh uninstall.sh

#####Arduino
######1、把`ArduinoLibrary` 放置到`C:\Program Files (x86)\Arduino\libraries`下.
######2、并更新库，依次点击`项目-->管理库`，IDE会自动更新库，并在选择框里输入`wilddog`，出现下图说明库安装成功.
######3、现在你可以到`C:\Program Files (x86)\Arduino\libraries\Wilddog\examples\ArduinoYun`下查看库的使用范例。
