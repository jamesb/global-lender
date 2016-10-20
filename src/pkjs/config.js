module.exports = [
  {
    "type": "heading",
    "defaultValue": "Global Lender"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "input",
        "messageKey": "LENDER_ID",
        "defaultValue": "",
        "label": "Kiva Lender ID",
        "description": "http://www.kiva.org/myLenderId",
        "attributes": {
          "placeholder": "eg: jeremy",
        }
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];