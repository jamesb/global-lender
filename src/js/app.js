
// Country Codes
var kivaCC = {};

var baseKivaUrl = "http://api.kivaws.org/v1/";
var jsonExt = ".json";
var kivaAppId = 'com.magnosity.global-lender';
var kivaAppIdParam = "appId=" + kivaAppId;
var lenderId = 'jdb';


// Global variable to store results from multi-page API calls
// Stored with keys equal to the page number (eg. range = [1 .. n pages])
var pageArray = [];


/////////////////////////////////////////////////////////////////////////////
/// Array extension to return the unique values of an array.
/////////////////////////////////////////////////////////////////////////////
Array.prototype.unique = function() {
    var a = [], l = this.length;
    for(var i=0; i<l; i++) {
      for(var j=i+1; j<l; j++)
            if (this[i] === this[j]) j = ++i;
      a.push(this[i]);
    }
    return a;
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
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



/////////////////////////////////////////////////////////////////////////////
/// Calls the Kiva API web service, parsing results by calling the function
/// passed in as parseFxn.
/// @param[in]      url  URL of the webservice
/// @param[in]      parseFxn  Function that returns a dictionary of
///       key/value pairs that represent the information that must be
///       passed back to the Pebble watch. Function should expect to receive
///       an array of JSON objects (range = [0 .. n-1 pages]), one value for
///       each page.
/// @param[in]      maxResults  Predetermined limit to the number of results
///       we need to fetch from the webservice. If maxResults is zero, then
///       all results will be fetched. (If zero results is really desired,
///       then don't call this function!)
/////////////////////////////////////////////////////////////////////////////
function callKivaApiAsync(url, parseFxn, maxResults) {
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
          var pageSize = parseInt(json.paging.page_size, 10);
          var pageTotal = parseInt(json.paging.pages, 10);

          var pageLimit;
          if (maxResults == 0) pageLimit = pageTotal;
          else pageLimit = Math.ceil(maxResults / pageSize);

          console.log("Received page " + pageNum + " of " + pageLimit + " (" + pageTotal + ")");
          pageArray[pageNum] = responseText;

          // Check if we have received all pages yet (up to limit).
          for (var pageIter=1; pageIter<=pageLimit; pageIter++) {
            if (pageIter in pageArray) {
              jsonPageArray[pageIter-1] = pageArray[pageIter];
            } else {
              allReceived = false;
              // If we have already requested paging in our URL, then don't make more requests.
              if (!url.match(/\&page=/)) {
                callKivaApiAsync(url + "&page=" + pageIter, parseFxn, maxResults);
              }
            }
          }
          // If we haven't received all pages, then return so that we don't send data to Pebble prematurely.
          if (!allReceived) return;
        } // end else

        // Parse results, send to Pebble, and reset the pageArray variable.
        console.log("Ready to parse JSON array of size " + Object.keys(jsonPageArray).length);
        dictionary = parseFxn(jsonPageArray);
        // Print all key pairs
        for (var key in dictionary) { if (dictionary.hasOwnProperty(key)) console.log(key + " -> " + dictionary[key]); }

        // JRB TODO: Need to check max message size and (if message exceeds
        // size), send dictionary in chunks
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log("Data sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending data to Pebble!");
          }
        );
        console.log("Clearing page array...");
        pageArray = [];
      }
    } // end embedded function
  ); // end xhrRequest
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
function getLenderInfo() {
  var url = baseKivaUrl + "lenders/" + lenderId + jsonExt + "?" + kivaAppIdParam;
  var maxResults = 0;

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

  callKivaApiAsync(url, parseFxn, maxResults);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
function getLoansForLender() {
  var url = baseKivaUrl + "lenders/" + lenderId + "/loans" + jsonExt + "?" + kivaAppIdParam;
  var maxResults = 0;

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

      var pageLimit;
      if (maxResults == 0) pageLimit = pageTotal;
      else pageLimit = Math.ceil(maxResults / pageSize);

      console.log("Parsing page " + pageNum + " of " + pageLimit + " (" + pageTotal + ")");

      lenderLoanQty = json.paging.total;

      // Iterate through each loan.
      for (var lidx = 0; lidx < pageSize && (pageSize * pageIter + lidx) < lenderLoanQty; lidx++) {
        var countryCode = json.loans[lidx].location.country_code;
        var countryName = json.loans[lidx].location.country;
        lenderCC[countryCode] = countryName;
        if (!(countryCode in kivaCC)) {
          // JRB TODO: store the country ID and name in a delta list for sending to the watch.
          kivaCC[countryCode] = countryName;
          console.log("(LENDER LOAN) NEW KIVA COUNTRY CODE: " + countryCode + " = " + countryName);
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

  callKivaApiAsync(url, parseFxn, maxResults);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
function getKivaActiveFieldPartners() {
  var url = baseKivaUrl + "partners" + jsonExt + "?" + kivaAppIdParam;
  var maxResults = 0;

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

      var pageLimit;
      if (maxResults == 0) pageLimit = pageTotal;
      else pageLimit = Math.ceil(maxResults / pageSize);

      console.log("Parsing page " + pageNum + " of " + pageLimit + " (" + pageTotal + ")");

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
            console.log("(PARTNER) NEW KIVA COUNTRY CODE: " + countryCode + " = " + countryName);
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

  callKivaApiAsync(url, parseFxn, maxResults);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
function getPreferredLoans(prefCC, maxResults) {
  var url = baseKivaUrl + "loans/search" + jsonExt + "?" + kivaAppIdParam + "&status=fundraising&country_code=" + prefCC;
  console.log("URL: " + url);

  var parseFxn = function(jsonPageArray) {
    var dictionary = {};
    var json = "";
    // pageNum range = [1 .. n pages];   pageIter range = [0 .. n-1 pages]
    var pageNum, pageSize, pageTotal, pageIter;
    var loansFlat = "";
    var loanId, name, use, countryCode, fundedAmt, loanAmt;

    for (pageIter=0; pageIter < Object.keys(jsonPageArray).length; pageIter++) {
      json = JSON.parse(jsonPageArray[pageIter]);

      pageNum = parseInt(json.paging.page, 10);
      pageSize = parseInt(json.paging.page_size, 10);
      pageTotal = parseInt(json.paging.pages, 10);

      var pageLimit;
      if (maxResults == 0) pageLimit = pageTotal;
      else pageLimit = Math.ceil(maxResults / pageSize);

      console.log("Parsing page " + pageNum + " of " + pageLimit + " (" + pageTotal + ")");

      loanQty = json.paging.total;

      // Iterate through each loan.
      for (var lidx = 0; lidx < Math.min(maxResults, pageSize) && (pageSize * pageIter + lidx) < loanQty; lidx++) {
        loanId = json.loans[lidx].id;
        name = json.loans[lidx].name;
        use = json.loans[lidx].use;
        countryCode = json.loans[lidx].location.country_code;
        fundedAmt = json.loans[lidx].funded_amount;
        loanAmt = json.loans[lidx].loan_amount;

        loansFlat = loansFlat +
            loanId +      "|" +
            name +        "|" +
            use +         "|" +
            countryCode + "|" +
            fundedAmt +   "|" +
            loanAmt +     "|";
      }

    } // end page iteration


    loansFlat = loansFlat.substring(0, loansFlat.length - 1);

    // Assemble dictionary using our keys
    dictionary = {
      "KEY_LOAN_SET" : loansFlat
    };

    return dictionary;
  }; // end parseFxn

  callKivaApiAsync(url, parseFxn, maxResults);
}


/////////////////////////////////////////////////////////////////////////////
/// Listen for when the watch opens communication and inform the watch that
/// the PebbleKit end of the channel is ready.
/////////////////////////////////////////////////////////////////////////////
Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");
    Pebble.sendAppMessage({"KEY_PEBKIT_READY": 1});

    getKivaActiveFieldPartners();
  }
);


/////////////////////////////////////////////////////////////////////////////
/// Listen for when an AppMessage is received.
/////////////////////////////////////////////////////////////////////////////
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received: " + JSON.stringify(e.payload));
    var dict = e.payload;

    if ('KEY_GET_LENDER_INFO' in dict) {
      getLenderInfo();
      getLoansForLender();
    } else if ('KEY_GET_PREFERRED_LOANS' in dict) {
      var prefCC = dict['KEY_GET_PREFERRED_LOANS'];
      var maxResults = 10;
      getPreferredLoans(prefCC, maxResults);
    } else {
      console.log("Unrecognized app message: " + JSON.stringify(dict));
    }


  }
);
