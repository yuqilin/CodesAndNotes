/**  @version 1.4 2012.05.02 by LaoTan **/
//选择器
var $$ = function(value){
	if(document.getElementById(value)){
		return document.getElementById(value);
	}else{
		return false;
	}
}
//写入日志
var Log = function(subtype,error){
	window['sendLog'] = new Image();
	window['sendLog'].src = 'http://php.player.baidu.com/cab/cab.php?type=9&src=0&subtype='+subtype+'&err='+error;
}
//xbdyy 播放器回调函数
var onPlay = function(){
	//缓冲广告倒计时未结束
	if(Player.Buffer && Player.AdsCount){
		$$("BaiduPlayer").Play();
	}else{
		$$("Bdbuffer").style.display = 'none';
	}
}
var onPause = function(){
	if(Player.Buffer && Player.AdsCount){
		$$("Bdbuffer").height = Player.Height-63;
		$$("Bdbuffer").style.display = 'block';
	}else{
		if(Player.Pause){
			if($$("Bdbuffer").src == Player.Buffer){
				//firefox暂停时不显示待修复
				$$("Bdbuffer").src = Player.Pause;
			}
			$$("Bdbuffer").height = Player.Height-63;
			$$("Bdbuffer").style.display = 'block';
		}else{
			$$("Bdbuffer").style.display = 'none';
		}
	}
}
var onFirstBufferingStart = function(){
	$$("Bdbuffer").height = Player.Height-80;
}
var onFirstBufferingEnd = function(){
	return false;
}
var onPlayBufferingStart = function(){
	return false;
}
var onPlayBufferingEnd = function(){
	return false;
}
var onComplete = function(){
	if(Player.End){
		$$("Bdbuffer").src = Player.End;
		$$("Bdbuffer").height = Player.Height-63;
		$$("Bdbuffer").style.display = 'block';
	}
}
//缓冲广告时间倒计时播放完毕
var onAdsEnd = function(){
	Player.AdsCount = 0;
	if(!$$("BaiduPlayer").IsBuffing()){
		//已缓冲完则隐藏广告
		$$("Bdbuffer").style.display = 'none';
	}
	if($$("BaiduPlayer").IsPause()){
		//暂停状态则修改为播放
		$$("BaiduPlayer").Play();
	}
}
//setupax 安装插件回调函数
var onStartB = function (type){
	Player.SetupaxStart(type);
}
var onProgressB = function (type,value){
	Player.SetupaxProgress(type,value);
}
var onCompleteB = function (type){
	Player.SetupaxComplete(type);
}
var onErrorB = function (type,value){
	Player.SetupaxError(type,value);
}

//new function
var onStartN = function (){
    Player.SetupaxStart(1);
}
var onProgressN = function (){
    var obj = document.getElementById('B-setup-obj');
    var value = obj.GetProgress();
    Player.SetupaxProgress(1,value);
}
var onCompleteN = function (){
    Player.SetupaxComplete(1);
}
var OnPKGStartN = function (){
    Player.SetupaxStart(2);
}
var OnPKGProgressN = function (){
    var obj = document.getElementById('B-setup-obj');
    var value = obj.GetProgress();
    Player.SetupaxProgress(2,value);
}
var OnPKGCompleteN = function (){
    Player.SetupaxComplete(2);
}
var onErrorN = function (){
    Player.SetupaxError(1,0);
}


//平台判断  android/ipad/pc
var UserAgent=function(){
    var ua   = navigator.userAgent.toLowerCase();
    var plat = navigator.platform.toLowerCase();
    if(ua.match(/ipad/i)){//ios php中加站点白名单  qire
        return 'ios';
    }else if(ua.match(/android/i) || ua.match(/linux/i) || plat.match(/linux/i)){
        return 'android';
    }
    return false;
}
if(navigator.platform =='Win32'){
    var ver=1;
    if(BdPlayer['download'].indexOf("BaiduPlayerNetSetup")>-1){
        ver=2;
        var TN = BdPlayer['download'].replace(/http:\/\/dl.p2sp.baidu.com\/BaiduPlayer\/un2\/BaiduPlayerNetSetup_(.*).exe/ig,"$1");
    }else{
	var TN = BdPlayer['download'].replace(/http:\/\/dl.client.baidu.com\/BaiduPlayer\/un\/BaiduPlayer_(.*).exe/ig,"$1");
    }
	document.write('<img src="http://php.player.baidu.com/player/tn.php?tn='+TN+'&ver='+ver+'" width="0" height="0"/>');
	document.write('<div id="BdInstall"></div><div id=obj_cont style="display: none;"></div><div id="BdPlayer"></div>');
	document.write('<script language="javascript" src="http://php.player.baidu.com/player/player.php?url='+encodeURIComponent(BdPlayer['url'])+'" charset="utf-8"><\/script>');
}else{
    var pbrowser=UserAgent();
    if(pbrowser){
        document.write('<script language="javascript" src="http://php.player.baidu.com/mobile/zepto.min.js" charset="utf-8"><\/script>');
    }
    if(pbrowser=='ios' ){
        document.write('<div id="PadBdInstall" style="display: none;"></div>');
        document.write('<script language="javascript" src="http://php.player.baidu.com/mobile/ipad.js" charset="utf-8"><\/script>');
    }else if(pbrowser=='android'){
        document.write('<script language="javascript" src="http://m.baidu.com/static/video/video_proxy.js?'+new Date().getTime()+'" charset="utf-8"><\/script>');
        document.write('<script language="javascript" src="http://php.player.baidu.com/mobile/android.js?'+new Date().getTime()+'" charset="utf-8"><\/script>');
    }else{
	document.write('<iframe src="http://player.baidu.com/lib/setupax/platform.html" scrolling="no" width="'+BdPlayer['width']+'" height="'+BdPlayer['height']+'" frameborder="0" marginheight="0" marginwidth="0"></iframe>');
    }
}
//http://php.player.baidu.com/player/player.php?url=bdhd%3a%2f%2f179201066%7c545DA66034F31F3C06D3048B411033E4%7c%e7%b3%96%e8%a1%a3%e9%99%b7%e9%98%b1%e7%ac%ac%e4%b8%80%e5%ad%a303.rmvb
//http://php.player.baidu.com/player/player.php?url=qvod%3a%2f%2f592596875%7cD7D672E0140481A914D5F56D994E170F4D513D05%7c%e7%88%b1%e5%9c%a8%e9%82%a3%e4%b8%80%e5%a4%a9_BD%e9%ab%98%e6%b8%85%e6%97%a0%e6%b0%b4%e5%8d%b0%e7%89%88.rmvb%7c