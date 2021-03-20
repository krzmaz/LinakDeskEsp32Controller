#pragma once
#include "DeskController.h"

extern LinakDesk::DeskController controller;
extern char deskName [32];

namespace html {
const char index_html[] =
    R"=====(<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN"><html lang="en" class=""> <meta charset="utf-8"/><meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no"/><title>$DeskName$ - Main Menu</title ><style>div, fieldset, input, select{padding: 5px; font-size: 1em;}select{width: 100%; background: #dddddd; color: #000000;}body{text-align: center; font-family: verdana, sans-serif; background: #252525;}td{padding: 0px;}button{border: 0; border-radius: 0.3rem; background: #1fa3ec; color: #faffff; line-height: 2.4rem; font-size: 1.2rem; width: 100%; -webkit-transition-duration: 0.4s; transition-duration: 0.4s; cursor: pointer;}button:hover{background: #0e70a4;}.bred{background: #d43535;}.bred:hover{background: #931f1f;}a{color: #1fa3ec; text-decoration: none;}.grid-container{display: grid; grid-template-columns: auto auto auto; grid-gap: 10px; padding: 10px;}.wide, .label{grid-column-start: 1; grid-column-end: 4;}.label{text-align: left; font-weight: normal; font-size: 20px;}</style> <script>function httpGet(theUrl){var xmlHttp=new XMLHttpRequest(); xmlHttp.open("GET", theUrl, false); xmlHttp.send(null); return xmlHttp.responseText;}setInterval(function (){getHeight();}, 2000); function getHeight(){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (this.readyState==4 && this.status==200){document.getElementById("heightDisplay").innerHTML=this.responseText + "mm";}}; xhttp.open("GET", "getHeightMm", true); xhttp.send();}</script> <style type="text/css"> div.c6{text-align: left; display: inline-block; color: #eaeaea; min-width: 340px;}div.c5{text-align: right; font-size: 11px;}a.c4{color: #aaa;}div.c3{width: 100%; padding: 0px;}div.c2{text-align: center; font-weight: normal; font-size: 62px;}div.c1{text-align: center; color: #eaeaea;}</style> <div class="c6"> <div class="c1"> <noscript >To use LinakDeskEsp32Controller, please enable JavaScript<br/></noscript> <h2>$DeskName$</h2> </div><div class="grid-container c3"> <div class="label">Current height:</div><div class="wide c2" id="heightDisplay">$Height$mm</div><form onsubmit="httpGet('moveToHeightMm?destination=$Pos1$')"> <button>$Pos1$mm</button> </form> <form onsubmit="httpGet('moveToHeightMm?destination=$Pos2$')"> <button>$Pos2$mm</button> </form> <form onsubmit="httpGet('moveToHeightMm?destination=$Pos3$')"> <button>$Pos3$mm</button> </form> <div> <a onclick="httpGet('saveCurrentPosAsFav?position=1')" href="." >Save current</a > </div><div> <a onclick="httpGet('saveCurrentPosAsFav?position=2')" href="." >Save current</a > </div><div> <a onclick="httpGet('saveCurrentPosAsFav?position=3')" href="." >Save current</a > </div><div class="label"></div><div class="label"> Go to: <input id="dest" type="number" min="0" max="1500" value="$Height$"/>mm </div><div class="wide"> <form onsubmit="httpGet('moveToHeightMm?destination='+document.getElementById('dest').value)" > <button>Go</button> </form> </div><div class="wide"><hr/></div><div class="wide"> <form action="cn" method="get" onsubmit='return confirm("Confirm Restart");' > <button class="button bred">Reboot to Config mode</button> </form> </div></div><div class="c5"> <hr/> <a href="https://github.com/TheRealMazur/LinakDeskEsp32Controller" target="_blank" class="c4" >LinakDeskEsp32Controller by Krzysztof (Chris) Mazur</a > </div></div></html>)=====";
String processor(const String& var) {
    if (var == "DeskName") {
        return String(deskName);
    }
    if (var == "Pos1") {
        return String(controller.getMemoryPositionMm(1).value_or(0) / 10);
    }
    if (var == "Pos2") {
        return String(controller.getMemoryPositionMm(2).value_or(0) / 10);
    }
    if (var == "Pos3") {
        return String(controller.getMemoryPositionMm(3).value_or(0) / 10);
    }
    if (var == "Height") {
        return String(controller.getHeightMm());
    }
    return String("");
}
} // namespace html
