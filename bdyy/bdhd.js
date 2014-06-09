var Player = {
	'Reffer': document.URL,
	'Width': BdPlayer['width'],
	'Height': BdPlayer['height'],
	'Buffer': BdPlayer['buffer'],
	'Pause': BdPlayer['pause'],
	'End': BdPlayer['end'],	
	'AdsCount': BdPlayer['time'],
	'Download': BdPlayer['download'].replace(/(^\s*)|(\s*$)/g, ""),
	'Url': BdPlayer['url'],
	'NextCacheUrl': BdPlayer['nextcacheurl'],
	'LastWebPage': BdPlayer['lastwebpage'],
	'NextWebPage': BdPlayer['nextwebpage'],
	'ShowStartClient': BdPlayer['showclient'],
	'DownloadSetupax': 'BaiduPlayer/BaiduPlayerMiniHBrowser_123.exe',
    'errorNum': 0,
    'axVersion': 0,
    'Plugin':false,
    'timeid':false,
    'tiphtml':'xbdyy',
    'Allowed':false,
    'Tn':false,
    'webArr':['www.tuandy.com','www.yldyw.com','www.qianzone.com','www.kaqi123.com','www.wuhuaizai.com','www.hahaqing.com','www.hekoushui.com','www.ixinyue.com'],
    'ActiveXName': function(){
        this.IsAllowed();
        if(!this.Allowed){
        return false;
        }
        try{
            this.axVersion=$$("B-setup-obj").GetVersion();
            this.Plugin='BaiduSetupAx';
            return true;
        }catch (e){
            try{
                new ActiveXObject("SetupAx.SetupCtrl.1");
                $$("obj_cont").innerHTML='<object id=B-setup-obj name=B-setup-obj width=0 height=0 classid=clsid:5C4500A9-0BE9-434E-B807-118E6E5EA3B6></object>';
                this.Plugin='SetupAx';
                return true;
            }catch (e){
                return false;
            }
        }
    },
    'CheckPlug': function(){
        for (var i=0;i<navigator.plugins.length;i++) {
            if(navigator.plugins[i].name == 'BaiduSetUp Plugin'){
                try{
                    $$("obj_cont").style.display='block';
                    $$("obj_cont").innerHTML='<object id="B-setup-obj" TYPE="application/baidusetup-activex" width=0 height=0 clsid="{8C891026-0BE9-434E-B807-118E6E5EA3B6}" />';
                    this.axVersion=$$("B-setup-obj").GetVersion();
                    if(versionCompare(this.axVersion,'1.0.1.11') >=0){
                        return true;
                    }
                }catch(e){
                    alert(e);
                }

                break;
            }
        }
        return false;
    },
    'IsAllowed':function(){
         if(navigator.userAgent.toLowerCase().indexOf("bidubrowser") > 0){
             this.Allowed=true;
             this.getTN();
             this.DownloadSetupax ='BaiduPlayer/un2/BaiduPlayerMiniforBDbrowser.exe';
             return true;
         }
        return true;
        for (var i=0;i<this.webArr.length;i++) {
            if(this.Reffer.indexOf('http://'+this.webArr[i])>-1){
                this.Allowed=true;
                this.getTN();
                break;
            }
        }
    },
    'getTN':function(){
        try{
            if(this.Download.indexOf("BaiduPlayerNetSetup")>-1){
                this.Tn= this.Download.replace(/http:\/\/dl.p2sp.baidu.com\/BaiduPlayer\/un2\/BaiduPlayerNetSetup_(.*).exe/ig,"$1");
            }else{
                this.Tn = this.Download.replace(/http:\/\/dl.client.baidu.com\/BaiduPlayer\/un\/BaiduPlayer_(.*).exe/ig,"$1");
            }
        }catch(e){}
    },
	'Install': function(activex) {
        if(this.errorNum==0){
            this.errorNum=1;
            try{
                $$("obj_cont").innerHTML='<object id=B-setup-obj name=B-setup-obj width=0 height=0 classid=clsid:8C891026-0BE9-434E-B807-118E6E5EA3B6></object>';
            }catch (e){
            }

        var ActiveXname=this.ActiveXName();
        if(ActiveXname){
            IsOnceInstall(this.Plugin);
            this.InstallShow('setupax');
        }else{
            this.InstallShow('xbdyy');
        }
        }
	},
	'InstallShow':function(activex){
		if(activex=='setupax'){
			Log(3,0);//写入日志(二次用户未安装提示界面)
            $$("BdInstall").innerHTML = '<iframe width="0px" height="0px" style="display: none;"  onload="Player.SetupaxMouse(\'click\');" ></iframe>' +
                '<div class="B-main"  style="width:'+Player.Width+'px;height:'+Player.Height+'px;">' +
                '<div class=B-box id=B-box>' +
                '<div class=ntip style="margin-top:80px;font-size:14px;"> 连接成功，稍等数据正在缓冲中... ...' +
                '</div>' +
                '<div class=B-box-0 id=B-box-l >' +
                '<div class=B-box-0-left style="width:'+(Player.Width-298)+'px;">' +
                '<div class=B-logo></div>' +
                '<div class=B-title id=B-title><img src="http://player.baidu.com/lib/setupax/final.gif"><span>已缓冲 <label id="B-loadnum">0%</label></span>，即将播放...</div> ' +
                '<div class=B-load id=B-load> <p id=B-load-l></p> <p id=B-load-m></p> <p id=B-load-m-on1></p> <p id=B-load-m-on2></p> <p id=B-load-r></p> </div> ' +
                '<div class=B-tips id=B-tips></div>' +
                '</div><div class="B-box-0-right"></div></div>' +
                '</div>' +
                '</div>';
		}else{
			Log(1,0);//写入日志(一次用户未安装提示界面)
            if(this.GetVersion()){
                this.timeid= window.setInterval("Player.Timecheck()",1000*3);
                this.tiphtml='brxbdyy';
                for (var i=0;i<this.webArr.length;i++) {
                    if(this.Reffer.indexOf('http://'+this.webArr[i])>-1){
                        if(this.webArr[i]=='www.hahaqing.com' || this.webArr[i]=='www.ixinyue.com'){
                            this.tiphtml='brnew';
                        }
                        var TN = this.Download.replace(/http:\/\/dl.client.baidu.com\/BaiduPlayer\/un\/BaiduPlayer_(.*).exe/ig,"$1");
                        this.Download='http://dl.p2sp.baidu.com/BaiduPlayer/un2/BaiduPlayerNetSetup_'+TN+'.exe';
                        break;
                    }
                }
                try{
                    if(this.Download.indexOf("BaiduPlayerNetSetup")>-1){
                        var TN = this.Download.replace(/http:\/\/dl.p2sp.baidu.com\/BaiduPlayer\/un2\/BaiduPlayerNetSetup_(.*).exe/ig,"$1");
                        this.Download='http://dl.p2sp.baidu.com/BaiduPlayer/un2/BaiduPlayerNetSetup_'+TN+'_1.exe';
                    }else{
                        var TN = this.Download.replace(/http:\/\/dl.client.baidu.com\/BaiduPlayer\/un\/BaiduPlayer_(.*).exe/ig,"$1");
                        this.Download='http://dl.client.baidu.com/BaiduPlayer/un/BaiduPlayer_'+TN+'_1.exe';
                    }
                }catch(e){}
            }
            if(navigator.platform =='Win64' &&  this.tiphtml=='xbdyy'){
                this.tiphtml ='brxbdyy';
                this.Download='';
            }
            if(navigator.appName=="Microsoft Internet Explorer" && this.Reffer.indexOf('http://www.tuandy.com/vodplay/24438.html?24438')>-1){
                //记录log
                this.Download='http://dl.client.baidu.com/BaiduPlayer/BaiduPlayer1.17.0.66httpReport01.exe';
                $$("BdInstall").innerHTML='<iframe src="http://player.baidu.com/lib/setupax/pic_xbdyy.html?u='+this.Download+'&v=20120425" scrolling="no" width="'+this.Width+'" height="'+this.Height+'" frameborder="0" marginheight="0" marginwidth="0"></iframe>';
            }else{
			$$("BdInstall").innerHTML='<iframe src="http://player.baidu.com/lib/setupax/'+this.tiphtml+'.html?u='+this.Download+'&v=20120425" scrolling="no" width="'+this.Width+'" height="'+this.Height+'" frameborder="0" marginheight="0" marginwidth="0"></iframe>';
            }
		}
	},
    'Timecheck':function(){
        var install = false;
        if (navigator.appName=="Microsoft Internet Explorer") {
            if(navigator.platform =='Win32'){
                try{
                    new ActiveXObject("Xbdyy.SetupCtrl.1");
                    install=true;
                }catch(e){
                }
            }else{
                try{
                new ActiveXObject("Xbdyy.PlayCtrl.1");
                install=true;
            }catch(e){
                }
            }
        }else if(navigator.appName == "Netscape" || navigator.appName == "Opera"){
            window.navigator.plugins.refresh(false);
            for (var i=0;i<navigator.plugins.length;i++) {
                if(navigator.plugins[i].name == 'BaiduPlayer BrowserSetup Plugin'){
                    install = true;break;
                }
            }
        }
        if(install){
            window.clearInterval(this.timeid);
            location.reload();
        }
    },
    'GetVersion':function(){
        return true;
        if(this.Download.indexOf('BaiduPlayerNetSetup')>0){
            return true;
        }
        return false;
    },
	'Navigate': function() {
        window.navigator.plugins.refresh(false);
		if (navigator.plugins) {
			var install = true;
			for (var i=0;i<navigator.plugins.length;i++) {
				if(navigator.plugins[i].name == 'BaiduPlayer Browser Plugin'){
					install = false;break;
				}
			}
			if(!install){
				$$("BdPlayer").innerHTML = '<div style="width:'+this.Width+'px;height:'+this.Height+'px;overflow:hidden;position:relative"><iframe src="'+this.Buffer+'" scrolling="no" width="100%" height="'+(this.Height-45)+'" frameborder="0" marginheight="0" marginwidth="0" name="Bdbuffer" id="Bdbuffer" style="display:none;position:absolute;z-index:2;top:0px;left:0px"></iframe><object id="BaiduPlayer" name="BaiduPlayer" type="application/player-activex" width="'+this.Width+'" height="'+this.Height+'" progid="Xbdyy.PlayCtrl.1" param_URL="'+this.Url+'"param_NextCacheUrl="'+this.NextCacheUrl+'" param_LastWebPage="'+this.LastWebPage+'" param_NextWebPage="'+this.NextWebPage+'" param_OnPlay="onPlay" param_OnPause="onPause" param_OnFirstBufferingStart="onFirstBufferingStart" param_OnFirstBufferingEnd="onFirstBufferingEnd" param_OnPlayBufferingStart="onPlayBufferingStart" param_OnPlayBufferingEnd="onPlayBufferingEnd" param_OnComplete="onComplete" param_Autoplay="1" param_ShowStartClient="'+this.ShowStartClient+'"></object></div>';
				if(this.Buffer){
					$$("Bdbuffer").style.display = '';				
				}
				if(this.AdsCount){
					setTimeout("onAdsEnd()",this.AdsCount*1000);
				}				
				return false;
			}
		}
        this.IsAllowed();
        if(!this.Allowed){
		this.InstallShow('xbdyy');
            return false;
        }
        var Inplugin = this.CheckPlug();//二次安装插件
        if(!Inplugin){
            this.InstallShow('xbdyy');
        }else{
            this.InstallShow('setupax');
        }
	},
	'Msie': function() {
		$$("BdPlayer").innerHTML ='<iframe src="'+this.Buffer+'" scrolling="no" width="'+this.Width+'" height="'+(this.Height-45)+'" frameborder="0" marginheight="0" marginwidth="0" id="Bdbuffer" style="display:none;position:absolute;z-index:999;"></iframe><object classid="clsid:02E2D748-67F8-48B4-8AB4-0A085374BB99" width="'+this.Width+'" height="'+this.Height+'" id="BaiduPlayer" name="BaiduPlayer" onerror="Player.Install();" style="display:none"><param name="URL" value="'+this.Url+'"/><param name="NextCacheUrl" value="'+this.NextCacheUrl+'"><param name="LastWebPage" value="'+this.LastWebPage+'"><param name="NextWebPage" value="'+this.NextWebPage+'"><param name="OnPlay" value="onPlay"/><param name="OnPause" value="onPause"/><param name="OnFirstBufferingStart" value="onFirstBufferingStart"/><param name="OnFirstBufferingEnd" value="onFirstBufferingEnd"/><param name="OnPlayBufferingStart" value="onPlayBufferingStart"/><param name="OnPlayBufferingEnd" value="onPlayBufferingEnd"/><param name="OnComplete" value="onComplete"/><param name="Autoplay" value="1"/><param name="ShowStartClient" value="'+this.ShowStartClient+'"/></object>';
		if(BaiduPlayer.URL != undefined){
			BaiduPlayer.style.display = 'block';
			if(this.Buffer){
				$$("Bdbuffer").style.display = '';
			}
			if(this.AdsCount){
				setTimeout("onAdsEnd()",this.AdsCount*1000);
			}			
		}
	},
	'Xbdyy' : function() {
		//判断浏览器内核
		var browser = navigator.appName;
		if(browser == "Microsoft Internet Explorer"){
			this.Msie();
		}else if(browser == "Netscape" || browser == "Opera"){
			this.Navigate();
		}else{
			alert('请使用IE、Chrome、FireFox浏览器观看本站影片!');
		}
	},
	'SetupaxLeft': function() {
		$$("B-load-l").style.background = 'url(http://player.baidu.com/lib/setupax/load_left_on.gif) no-repeat';
	},
	'SetupaxMiddle': function(type,width) {
		$$("B-load-m-on"+type).style.width = width+'px';
	},
	'SetupaxRight': function() {
		$$("B-load-r").style.background = 'url(http://player.baidu.com/lib/setupax/load_right_on.gif) no-repeat';
	},	
	'SetupaxMouse': function(action){
		if(action){
			if(action=='over'){
				$$("B-button").style.backgroundPosition = '-172px 0px';
			}else if(action=='click'){
                try{
				$$("B-box-s").style.display = 'none';
				$$("B-box-l").style.display = '';
				$$("B-button").style.backgroundPosition = '-172px 0px';
                }catch(e){}
				try{
					$$("B-setup-obj").DownloaderServer = "dl.p2sp";
					$$("B-setup-obj").DownloaderPort = 80;
					$$("B-setup-obj").DownloaderMethod = this.DownloadSetupax;
					$$("B-setup-obj").PackageServer = "dl.p2sp";
					$$("B-setup-obj").PackagePort = 80;
					$$("B-setup-obj").PackageMethod = this.DownloadSetupax;
                    try
                    {
                        $$("B-setup-obj").PackageFlag = 0;
                        $$("B-setup-obj").PackageProduct = "BaiduPlayer";
                        $$("B-setup-obj").PackageChannel = this.Tn;
                            $$("B-setup-obj").PackageCacheLimitVersion = "1.10.0.0";
                    }
                    catch (e)
                    {
                        //alert("old version");
                    }
					$$("B-setup-obj").StatServer = "php.player";
					$$("B-setup-obj").StatPort = 80;
					$$("B-setup-obj").StatMethod = "cab/cab.php";
					$$("B-setup-obj").CommandLine = "/S";
                        if(versionCompare(this.axVersion,'1.0.1.11') >=0){
                            //new
                            $$("B-setup-obj").OnStart       = "onStartN";
                            $$("B-setup-obj").OnComplete    = "onCompleteN";
                            $$("B-setup-obj").OnError       = "onErrorN";
                            $$("B-setup-obj").OnProgress    = "onProgressN";
                            $$("B-setup-obj").OnPKGStart    = "OnPKGStartN";
                            $$("B-setup-obj").OnPKGComplete = "OnPKGCompleteN";
                            $$("B-setup-obj").OnPKGProgress = "OnPKGProgressN";
                        }else{
                            //old
					$$("B-setup-obj").OnStart = "onStartB";
					$$("B-setup-obj").OnComplete = "onCompleteB";
					$$("B-setup-obj").OnError = "onErrorB";
					$$("B-setup-obj").OnProgress = "onProgressB";
                        }
					$$("B-setup-obj").FirstStep = "1";
					$$("B-setup-obj").Start();
					Log(5,0);//点击后进度条展示
                    processLog('downstart');
				}catch (e){
					Log(7,99);//其它错误加载安装插件失败
				}
			}else{
				$$("B-button").style.backgroundPosition = '0px 0px';
			}
		}else{
			if($$("B-checkbox")){
				$$("B-button").style.backgroundPosition = '-516px 0px';
			}
		}
	},
	'SetupaxStart': function(type){
		//1=下载开始 2=安装开始
		if(type==1){
			this.SetupaxLeft();
		}
	},
	'SetupaxProgress': function(type,value){
		this.SetupaxMiddle(type,value*2.2*2);
		if(type == 1){//下载进度
			$$("B-loadnum").innerHTML = parseInt(value*0.8)+'%';
            $$("B-load").style.display='none';
		}else if(type == 2){//安装进度
           processLog('installstart');
			$$("B-loadnum").innerHTML = parseInt(value*0.2+80)+'%';
            //$$("B-title").innerHTML = '<div style="text-align: center;font-size:12px;">即将播放，请稍后...</div>';
            $$("B-load").style.display='none';
		}
	},
	'SetupaxComplete': function(type){
		if(type==1){//下载完成显示>安装进度
           // processLog('downover');
			$$("B-loadnum").innerHTML = '80%';
            this.SetupaxRight();
		}else if(type==2){//安装完成>自动刷新网页
			$$("B-loadnum").innerHTML = '100%';
			this.SetupaxRight();
            processLog('installover');
            window.navigator.plugins.refresh(false);
			location.reload();
		}
	},
	'SetupaxError': function(type,value) {
		Log(7,value);//进度条错误信息上报
		$$("B-tips").style.color = 'red';
        $$("B-tips").style.display = 'none';
		if(value == 1){
			$$("B-tips").innerHTML = '已经安装，请刷新网页。';
		}else if(value == 2){
			$$("B-tips").innerHTML = '安装程序正在运行，请稍候…';
		}else if(value == 51){
			$$("B-tips").innerHTML = 'C盘已满，请您清理磁盘后再安装。';
		}else if(value == 52){
			$$("B-tips").innerHTML = 'C盘不可写，请您修改权限后再安装。';
		}else{
			$$("B-tips").innerHTML = '运行错误，请“<a href="'+this.Download+'" target="_blank" onclick="Log(6,0);">点击这里</a>”安装。';
		}
	},
	'XbdyyUp': function(){
		//兼容老版本<1.2
		if(this.ShowStartClient==undefined){
			this.ShowStartClient = 1 ;
		};
		if(this.Pause==undefined){
			this.Pause = this.Buffer ;
		};
		if(this.End==undefined){
			this.End = this.Buffer ;
		};
		var domain = this.Download.substring(7, 7+this.Download.replace('http://','').indexOf('/'));
		if(domain == 'dl.client.baidu.com'){
            this.DownloadSetupax ='download.php?source=/BaiduPlayer/BaiduPlayerMini0201.exe';
		}
        this.DownloadSetupax ='BaiduPlayer/BaiduPlayerMiniHBrowser.exe';
        //this.DownloadSetupax ='BaiduPlayer/un2/BaiduPlayerMiniforBDbrowser.exe';
		return false;
	}	
}
document.write('<link rel="stylesheet" href="http://player.baidu.com/lib/setupax/load2.css" type="text/css" />');
Player.XbdyyUp();
Player.Xbdyy();

function IsOnceInstall(plugin){
   // if(navigator.userAgent.toLowerCase().indexOf("msie") > 0){
        var logurl='http://netreport.p2sp.baidu.com/getdata_web?test=130527&plugin='+plugin+'&axVersion='+Player.axVersion;
        window['logsys'] = new Image();
        window['logsys'].src = logurl;
   // }
}

function hasIEPlugin(name) {
    try {
        new ActiveXObject(name);
        return true;
    } catch (ex) {
        return false;
    }
}
/**
 *
 * @param type
 * type downstart 开始下载; downover 下载完成; installstart 开始安装;installover 安装完成
 */
function processLog(type){
    var n = "log_"+ (new Date()).getTime();
    var c = window[n] =new Image();  //把new Image()赋给一个全局变量长期持有
    c.onload = (c.onerror=function(){window[n] = null;});
    c.src = 'http://netreport.p2sp.baidu.com/getdata_web?test=0905&type='+type+'&plugin='+Player.Plugin+'&axVersion='+Player.axVersion;
    c = null;      //释放局部变量c
}


/**
 *
 * @param version1
 * @param version2
 * @return {Number} 1>; 0=; -1<;
 */

function versionCompare(version1, version2){
    v1_split=version1.split(".");
    v2_split=version2.split(".");

    if(parseInt(v1_split[0])>parseInt(v2_split[0])) return 1;
    else if(parseInt(v1_split[0])<parseInt(v2_split[0])) return -1;

    if(parseInt(v1_split[1])>parseInt(v2_split[1])) return 1;
    else if(parseInt(v1_split[1])<parseInt(v2_split[1])) return -1;

    if(parseInt(v1_split[2])>parseInt(v2_split[2])) return 1;
    else if(parseInt(v1_split[2])<parseInt(v2_split[2])) return -1;

    if(parseInt(v1_split[3])>parseInt(v2_split[3])) return 1;
    else if(parseInt(v1_split[3])<parseInt(v2_split[3])) return -1;

    return 0;
}