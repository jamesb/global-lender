
// Country Codes
var Iso3166 = require('iso3166');
var iso3166 = Iso3166.getCountryCodes();
var kivaCC = {};

var baseKivaUrl = "http://api.kivaws.org/v1/";
var jsonExt = ".json";

var kivaAppId = 'com.magnosity.global-lender';
var lenderId = 'jdb';


// Global variable to store results from multi-page API calls
// Stored with keys equal to the page number (eg. range = [1 .. n pages])
var pages = [];


/**************************************************************************
 * Array extension to return the unique values of an array.
 **************************************************************************/
Array.prototype.unique = function() {
    var a = [], l = this.length;
    for(var i=0; i<l; i++) {
      for(var j=i+1; j<l; j++)
            if (this[i] === this[j]) j = ++i;
      a.push(this[i]);
    }
    return a;
};


/**************************************************************************
 **************************************************************************/
var xhrRequest = function (method, url, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.onerror = function() {
    console.log("HTTP " + xhr.status + " ERROR: " + xhr.statusText);
    // JRB TODO: Need to create a dictionary key to send an error message to Pebble for display to user.
  };
  xhr.ontimeout = function() {
    console.log("HTTP TIMEOUT for " + url);
    // JRB TODO: Need to create a dictionary key to send an error message to Pebble for display to user.
  };
  xhr.timeout = 10000;  // 10 sec
  xhr.open(method, url);
  xhr.send();
};

      
      
/**************************************************************************
 * Calls the Kiva API web service, parsing results by calling the function
 * passed in as parseFxn. 
 * Params:
 *   url:        URL of the webservice
 *   parseFxn:   Function that returns a dictionary of key/value pairs
 *       that represent the information that must be passed back to the
 *       Pebble watch. Function should expect to receive an array of
 *       JSON objects (range = [0 .. n-1 pages]), one value for each page.
 **************************************************************************/
function callKivaApiAsync(url, parseFxn) {
  // Send request
  xhrRequest('GET', url, function(responseText) {
      var json = JSON.parse(responseText);
      var jsonPageArray = [];
      var dictionary = {};
      var allReceived = true;
      if (json.code) {
        console.log("RESPONSE ERROR " + json.code + ": " + json.message);
        // JRB TODO: Need to create a dictionary key to send an error message to Pebble for display to user.
      } else {
        if (!json.paging) {
          // Single-page result processing
          jsonPageArray[0] = json;
        } else {
          // Multi-page result processing
          var pageNum = parseInt(json.paging.page, 10);
          var pageTotal = parseInt(json.paging.pages, 10);
          console.log("Received page " + pageNum + " of " + pageTotal);
          pages[pageNum] = responseText;
          
          // Check if we have received all pages yet.
          for (var pageIter=1; pageIter<=pageTotal; pageIter++) {
            if (pageIter in pages) jsonPageArray[pageIter-1] = pages[pageIter];
            else {
              allReceived = false;
              // If we have already requested paging in our URL, then don't make more requests.
              if (!url.match(/\?page=/)) callKivaApiAsync(url + "?page=" + pageIter, parseFxn);
            }
          }
          // If we haven't received all pages, then return so that we don't send data to Pebble prematurely.
          if (!allReceived) return;
        } // end else
        
        // Parse results, send to Pebble, and reset the pages variable.
        console.log("Ready to parse JSON array of size " + Object.keys(jsonPageArray).length);
        dictionary = parseFxn(jsonPageArray);
        // Print all key pairs
        for (var key in dictionary) { if (dictionary.hasOwnProperty(key)) console.log(key + " -> " + dictionary[key]); }
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log("Data sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending data to Pebble!");
          }
        );
        console.log("Clearing pages...");
        pages = [];
      }
    } // end embedded function
  ); // end xhrRequest
}


/**************************************************************************
 **************************************************************************/
function getLenderInfo() {
  var url = baseKivaUrl + "lenders/" + lenderId + jsonExt;

  var parseFxn = function(jsonPageArray) {
    var dictionary = {};
    if (Object.keys(jsonPageArray).length != 1) {
      console.log("ERROR: Only expected a single-page result for " + url);
      return;
    }
    var json = jsonPageArray[0];
    var lenderName = json.lenders[0].name;
    console.log("lenderName = " + lenderName);
    var lenderLoc = json.lenders[0].whereabouts;
    console.log("lenderLoc = " + lenderLoc);
    var lenderLoanQty = json.lenders[0].loan_count;
    console.log("Lender Loan Qty = " + lenderLoanQty);

    // Assemble dictionary using our keys
    dictionary = {
      "KEY_LENDER_ID"          : lenderId,
      "KEY_LENDER_NAME"        : lenderName,
      "KEY_LENDER_LOC"         : lenderLoc,
      "KEY_LENDER_LOAN_QTY"    : lenderLoanQty
    };
    return dictionary;
  };

  callKivaApiAsync(url, parseFxn);
}


/**************************************************************************
 **************************************************************************/
function getLoansForLender() {
  var url = baseKivaUrl + "lenders/" + lenderId + "/loans" + jsonExt;

  var parseFxn = function(jsonPageArray) {
    var dictionary = {};
    var json = "";
    // pageNum range = [1 .. n pages];   pageIter range = [0 .. n-1 pages]
    var pageNum, pageSize, pageTotal, pageIter;
    var lenderLoanQty;
    var lenderCC = {};
    
    for (pageIter=0; pageIter < Object.keys(jsonPageArray).length; pageIter++) {
      json = JSON.parse(jsonPageArray[pageIter]);
      
      pageNum = parseInt(json.paging.page, 10);
      pageSize = parseInt(json.paging.page_size, 10);
      pageTotal = parseInt(json.paging.pages, 10);
      console.log("Parsing Page " + pageNum + " of " + pageTotal);

      lenderLoanQty = json.paging.total;
      
      // Iterate through each loan. 
      for (var lidx = 0; lidx < pageSize && (pageSize * pageIter + lidx) < lenderLoanQty; lidx++) { 
        var countryCode = json.loans[lidx].location.country_code;
        var countryName = json.loans[lidx].location.country;
        lenderCC[countryCode] = countryName;
        if (!(countryCode in kivaCC)) {
          // JRB TODO: store the country ID and name in a delta list for sending to the watch.
          kivaCC[countryCode] = countryName;
          console.log("(LENDER LOAN) NEW KIVA COUNTRY CODE: " + countryCode + " = " + countryName + " { ISO-3166 = " + iso3166[countryCode] + " }");
        }
      }
    } // end page iteration
    
    // Assemble Lender's Kiva countries
    var lenderCCFlat = "";
    for (var key in lenderCC) { 
      if (lenderCC.hasOwnProperty(key)) {
        lenderCCFlat = lenderCCFlat + key + "|" + lenderCC[key] + "|";
      }
    }
    lenderCCFlat = lenderCCFlat.substring(0, lenderCCFlat.length - 1);
    
    // Assemble dictionary using our keys
    dictionary = {
      "KEY_LENDER_ID"          : lenderId,
      "KEY_LENDER_LOAN_QTY"    : lenderLoanQty,
      "KEY_LENDER_COUNTRY_SET" : lenderCCFlat
    };

    return dictionary;
  }; // end parseFxn

  callKivaApiAsync(url, parseFxn);
}


/**************************************************************************
 **************************************************************************/
function getKivaActiveFieldPartners() {
  var url = baseKivaUrl + "partners" + jsonExt;

  var parseFxn = function(jsonPageArray) {
    var dictionary = {};
    var json = "";
    // pageNum range = [1 .. n pages];   pageIter range = [0 .. n-1 pages]
    var pageNum, pageSize, pageTotal, pageIter;
    var partnerQty;
    var deltaKivaCC = {};
    
    for (pageIter=0; pageIter < Object.keys(jsonPageArray).length; pageIter++) {
      json = JSON.parse(jsonPageArray[pageIter]);
      
      pageNum = parseInt(json.paging.page, 10);
      pageSize = parseInt(json.paging.page_size, 10);
      pageTotal = parseInt(json.paging.pages, 10);
      console.log("Parsing Page " + pageNum + " of " + pageTotal);

      partnerQty = json.paging.total;
      
      // Iterate through each partner. 
      for (var pidx = 0; pidx < pageSize && (pageSize * pageIter + pidx) < partnerQty; pidx++) { 
        var partnerCountryQty = json.partners[pidx].countries.length;
        var partnerStatus = json.partners[pidx].status;
        
        for (var cidx=0; cidx < partnerCountryQty; cidx++) {
          var countryCode = json.partners[pidx].countries[cidx].iso_code;
          var countryName = json.partners[pidx].countries[cidx].name;
          if (!(countryCode in kivaCC) && partnerStatus.toLowerCase() === "active") {
            // JRB TODO: store the country ID and name in a delta list for sending to the watch.
            kivaCC[countryCode] = countryName;
            deltaKivaCC[countryCode] = countryName;
            console.log("(PARTNER) NEW KIVA COUNTRY CODE: " + countryCode + " = " + countryName + " { ISO-3166 = " + iso3166[countryCode] + " }");
          }
        }
      }
    
    } // end page iteration
    
    
    // Assemble Kiva countries not previously sent to watch
    var deltaKivaCCFlat = "";
    for (var key in deltaKivaCC) { 
      if (deltaKivaCC.hasOwnProperty(key)) {
        deltaKivaCCFlat = deltaKivaCCFlat + key + "|" + deltaKivaCC[key] + "|";
      }
    }
    deltaKivaCCFlat = deltaKivaCCFlat.substring(0, deltaKivaCCFlat.length - 1);
    
    // Assemble dictionary using our keys
    dictionary = {
      "KEY_KIVA_COUNTRY_SET" : deltaKivaCCFlat
    };

    return dictionary;
  }; // end parseFxn

  callKivaApiAsync(url, parseFxn);
}


/**************************************************************************
 * Listen for when the watch opens communication and inform the watch that
 * the PebbleKit end of the channel is ready.
 **************************************************************************/
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    Pebble.sendAppMessage({"KEY_PEBKIT_READY": 1});
    
    getKivaActiveFieldPartners();
  }
);


/**************************************************************************
 * Listen for when an AppMessage is received.
 **************************************************************************/
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    if ('KEY_GET_LENDER_INFO' in e.payload) {
      getLenderInfo();
      getLoansForLender();
    } else {
      console.log("Unrecognized app message: " + JSON.stringify(e.payload));
    }
  }                     
);
