var file = "-";
var sec_max = 70;
var timer_val = 200;
var seconds;
var percent;
function prepareUpload() {
	document.getElementById('fileSize').innerHTML = '';
	document.getElementById('bytesUploaded').innerHTML = '';
	document.getElementById('percentUploaded').innerHTML = '0%';
	document.getElementById('uploadProgressBar').style.width = '0%';

	// get file name
	var fileName = document.getElementById('file_upl').value;
	if (fileName.lastIndexOf('\\')>=0)
		fileName = fileName.substr(fileName.lastIndexOf('\\')+1);
	file = fileName;
	if (document.getElementById('upload_file_name'))
		document.getElementById('upload_file_name').innerHTML = fileName;
	// Размер файла
	var file_obj = document.getElementById("file_upl").files[0];
	var fSize = 0;
	if (file_obj) {
		fSize = file_obj.size;
	}
	document.getElementById('fileSize').innerHTML = fSize;
	// get folder path
	var curFolder = window.location.href;
	if(curFolder[curFolder.length-1]!='/')
		curFolder = curFolder.substring(0, curFolder.lastIndexOf('/')+1);

	document.getElementById('target').innerHTML = curFolder;
	document.getElementById('frm').action = curFolder;
	
	seconds = sec_max * 1000 / timer_val;
	percent = 0;
	return fileName;
}

var timerId;
function formSubmit() {
	if ((file) && (file.length > 10)) {
		timerId = setInterval('updateProgress()', timer_val);
		document.getElementById('cancelUploadBtn').disabled = false;
		return true;
	} else {
		alert("Файл не выбран!");
		return false;
	}
}

function checkIE() {
    var ua = window.navigator.userAgent;
    var msie = ua.indexOf("MSIE ");

    if (msie > 0 || !!navigator.userAgent.match(/Trident.*rv\:11\./)){  
        // If Internet Explorer, return version number
        // alert(parseInt(ua.substring(msie + 5, ua.indexOf(".", msie))));
		return true;
    } else {
        // If another browser, return 0
        // alert('otherbrowser');
		return false;
    }
}

function updateProgress() {
	if (percent < 100) {
		var request = window.ActiveXObject ? new ActiveXObject("Microsoft.XMLHTTP") : new XMLHttpRequest();
		var uploadTarget = document.getElementById("frm").action + file;
		var IsIE = checkIE();
		request.open("REPORT", uploadTarget, false);
		request.send("<upload-progress xmlns='ithit'/>");
		var resp = request.responseText;
		// Extract number of bytes uploaded and total content length of the file.
		// Usually you will use XML DOM or regular expressions for this purposes
		// but here for the sake of simplicity we will just extract using string methods.
		var size;
		var sizeIndex = resp.indexOf("total-content-length");
		if(sizeIndex != -1) {
			size = resp.substring(resp.indexOf(">", sizeIndex)+1, resp.indexOf("</", sizeIndex));
			document.getElementById("fileSize").innerHTML = size;
		} else {
			var file_obj = document.getElementById("file_upl").files[0];
			if (file_obj) {
				size = file_obj.size;
			}
		}
		var bytes;
		// percent = 100;
		var bytesIndex = resp.indexOf("bytes-uploaded");
		if(bytesIndex != -1)  {
			bytes = resp.substring(resp.indexOf(">", bytesIndex)+1, resp.indexOf("</", bytesIndex));
			if(parseInt(size)!=0)
				percent = 100*parseInt(bytes)/parseInt(size);
			document.getElementById("bytesUploaded").innerHTML = bytes;
		} else {
			var file_obj = document.getElementById("file_upl").files[0];
			if (file_obj) {
				document.getElementById('bytesUploaded').innerHTML = file_obj.size;
			}
		}
		document.getElementById("percentUploaded").innerHTML = percent.toString().substr(0, 4) + " %";
		document.getElementById("uploadProgressBar").style.width = percent.toString() + "%";
	}
	if(percent==100)  {
		document.getElementById("cancelUploadBtn").disabled = true;
		--seconds;
		var cPers = document.getElementById("percentUploaded");
		if (seconds > 0) {
			if (cPers) {
				var cSec = parseInt(seconds * timer_val / 1000);
				cPers.innerHTML = "!!! Обновление ПО (" + cSec.toString() + " сек)";
				cPers.style.color = 'red';
			}
		} else {
			clearInterval(timerId);
			if (cPers) {
				cPers.innerHTML = "Обновление завершено";
				cPers.style.color = 'black';
				this.submit();
			}
		}
	}
}

function cancelUpload() {
	// recreate iframe to cancel upload
	document.getElementById("uploadFrameHolder").innerHTML = "<iframe name='uploadFrame' ></iframe>";
	clearInterval(timerId);
	document.getElementById("cancelUploadBtn").disabled = true;
}

function window_onload() {
	var cPrs = document.getElementById("percentUploaded");
	cPrs.innerHTML = '0%';
}

