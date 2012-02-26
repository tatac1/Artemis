var http = require('http');
var url = require('url');
var path = require('path');
var ail = require('./ail-module/build/Release/ailreader');

var AILSchema;

function extractKeyset(assoArray) {
    if (assoArray.length == 0) {
	return {};
    } else {
	var keyset = new Array();
	var i = 0;
	for (key in assoArray) {
	    keyset[i] = key;
	    i = i + 1;
	}
	return keyset;
    }
}

function extractValues(assoArray) {
    if (assoArray.length == 0) {
	return {};
    } else {
	var values = new Array();
	var i = 0;
	for (key in assoArray) {
	    values[i] = assoArray[key];
	    i = i + 1;
	}
	return values;
    }
}

function trimEmpty(list) {
	var new_list = []

	for (i = 0; i < list.length; i++) {
		if (list[i] != '') {
			new_list.push(list[i]);
		}
	}

	return new_list;
}


function requestHandler(request, response) {
    
    var request_url = url.parse(request.url, true);
    var opArgs = trimEmpty(request_url.pathname.split('/'));
    var queryKeys = extractKeyset(request_url.query);
    var queryValues = extractValues(request_url.query);

    var ailResponse;

	request.addListener('data', function(chunk) {
	    // TODO handle post data
	    console.log('WARNING, unhandled post data detected in request')
	});

    ailResponse = ail.generate_response_permutation(opArgs, queryKeys, queryValues, AILSchema); 
   
    if (ailResponse != undefined) {
		console.log('AIL-response for ', request.url);

		request.addListener('end', function() {
		    
		    response.writeHead(200, {
		    'Content-Length' : ailResponse.length,
		    'Content-Type'   : 'application/json'});

			response.write(ailResponse);
			response.end();

			console.log("Response written");
		});
	
    } else {
		console.log('Proxy-response for ', request.url);
		
		target = request.headers['host'].split(':');
		hostname = target[0];
		port = (target.length > 1) ? target[1] : 80;

		var options = {
		    host: hostname,
		    port: port,
		    method: request.method, 
		    path: (request_url.pathname || '/') + (request_url.search || ''), 
		    headers: request.headers
		}

		var proxy_request = http.request(options, function(proxy_response) {

		    response.writeHead(proxy_response.statusCode, proxy_response.headers);
		    
		    proxy_response.addListener('data', function(chunk) {
				response.write(chunk, 'binary');
		    });

		    proxy_response.addListener('end', function() {
				response.end();
		    });
		    
		});

		request.addListener('data', function(chunk) {
		    proxy_request.write(chunk, 'binary');
		});

		request.addListener('end', function() {
		    proxy_request.end();
		});


		proxy_request.on('error', function(e) {
		    console.error('Error encountered handling URL: ' + request.url);
		    response.end();
		});
	
    }
}

if (process.argv.length != 3) {
    console.log('Error, proper usage: node ailproxy.js /path/to/schema');
} else {
    var AILSchema = process.argv[2];
    http.createServer(requestHandler).listen(8080);
    console.log('Launched AIL Proxy, listening on port 8080');
}