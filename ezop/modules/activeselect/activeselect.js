// $Id: activeselect.js,v 1.18 2007/01/05 15:57:53 jaza Exp $

/**
 * Attaches the activeselect behaviour to all required fields
 */
Drupal.activeselectAutoAttach = function () {
  var asdb = [];
  $('input.activeselect-path').each(function () {
    var index = this.id.substr(0, this.id.length - 18);
    var uri = this.value +'/'+ encodeURIComponent(index).substr(5);
    var extra = $('#' + index + '-activeselect-extra').val();
    var targets = $('#' + index + '-activeselect-targets').val();
    var input = $('#' + index).get(0);

    if (!asdb[uri]) {
      asdb[uri] = new Drupal.ASDB(uri, targets);
    }
    new Drupal.jsAS(input, asdb[uri], targets, extra);
  });
}

/**
 * An ActiveSelect object
 */
Drupal.jsAS = function (input, db, targets, extra) {
  var as = this;
  this.input = input;
  this.db = db;
  $(this.input).change(function (event) { return as.onchange(this, event); });
  this.extra = extra;
  var targetsArray = targets.split(',');
  this.targets = [];
  for (var target = 0; target < targetsArray.length; target++) {
    var newTarget = $('#' + targetsArray[target]).get(0);
    newTarget.owner = this;
    this.targets.push(newTarget);
  }
  // this only runs if the current element does not have a parent activeselect
  // linked to it - otherwise, IE has problems.
  if (!this.input.owner) {
    this.populateTargets();
  }
};

/**
 * Handler for the "onchange" event
 */
Drupal.jsAS.prototype.onchange = function (input, e) {
  if (!e) {
    e = window.event;
  }

  this.populateTargets();
}

/**
 * Return the currently selected options as a pipe-delimited string
 */
Drupal.jsAS.prototype.getSelectedOptions = function () {
  var selectedOptions = [];
  var maxWidth = 0;
  $('#' + this.input.id + ' option').each(function () {
    if (this.selected) {
      var optionString = this.value.replace(/\|/g, '&#124;') +'|'+ this.text.replace(/\|/g, '&#124;');
      selectedOptions.push(optionString);
    }
    if (this.text.length > maxWidth) {
      maxWidth = this.text.length;
    }
  });
  this.setSelectWidth(maxWidth);

  return selectedOptions.join('||');
}

/**
 * Sets the width and background position of the activeselect element.
 */
Drupal.jsAS.prototype.setSelectWidth = function (width) {
  if (width != null) {
    this.selectWidth = ((width * 10) * 1.5) + 20;
  }
  $(this.input).css({
    width: this.selectWidth +'px',
    backgroundPosition: (this.selectWidth - 35) +'px 2px'
  });
}

/**
 * Sets the width of the specified target element
 */
Drupal.jsAS.prototype.setTargetWidth = function (target, width) {
  if (width != null) {
    this.targets[target].targetWidth = (width * 10) * 1.2;
  }
  $(this.targets[target]).css({
    width: this.targets[target].targetWidth +'px'
  });
}

/**
 * Starts a search
 */
Drupal.jsAS.prototype.populateTargets = function () {
  var as = this;
  this.db.owner = this;

  this.db.search(this.getSelectedOptions(), this.targets, this.extra);
}

/**
 * Fills the target select boxes with any matches received
 */
Drupal.jsAS.prototype.populate = function (matches) {
  for (targetIndex in this.targets) {
    var target = this.targets[targetIndex];
    var matchesTarget = 0;
    for (targetElement in matches) {
      if ('edit-'+targetElement == target.id) {
        matchesTarget = targetElement;
        continue;
      }
    }
    if (matchesTarget) {
      this.targets[targetIndex].multiple = matches[matchesTarget]['multiple'];
      if (matches[matchesTarget]['multiple']) {
        if (target.name.indexOf('[]') == -1) {
          this.targets[targetIndex].name += '[]';
        }
      }
      else {
        var bracketIndex = target.name.indexOf('[]');
        if (bracketIndex != -1) {
          this.targets[targetIndex].name = target.name.substr(0, target.name.length-2);
        }
      }

      $(target).empty();
      var targetMatches = matches[matchesTarget]['options'];
      var maxWidth = 0;
      for (currMatch in targetMatches) {
        var value = currMatch;
        var text = targetMatches[currMatch]['value'];
        var selected = targetMatches[currMatch]['selected'];

        if (text.length > maxWidth) {
          maxWidth = text.length;
        }
        // 'new Option()' used instead of appendChild(), because IE6 refuses to
        // display option text if the latter method is used (otherwise they seem
        // to behave the same).
        $(this.targets[targetIndex]).append(new Option(text, value, false, selected));
        if (selected && !this.targets[targetIndex].multiple) {
          this.targets[targetIndex].selectedIndex = this.targets[targetIndex].options.length-1;
        }
      }
      if (this.targets[targetIndex].selectedIndex == -1) {
        this.targets[targetIndex].selectedIndex = 0;
      }

      if (this.hasClass(this.targets[targetIndex], 'form-activeselect')) {
        // Since IE does not support the DOM 2 methods for manually firing an
        // event, we must cater especially to its needs.
        // Reference: http://www.howtocreate.co.uk/tutorials/javascript/domevents
        if (document.createEvent) {
          // DOM 2 compliant method (Firefox / Opera / Safari / etc)
          var e = document.createEvent('HTMLEvents');
          e.initEvent('change', true, false);
          this.targets[targetIndex].dispatchEvent(e);
        }
        else if (document.createEventObject) {
          // IE special weird method
          var e = document.createEventObject();
          e.bubbles = true;
          e.cancelable = false;
          this.targets[targetIndex].fireEvent('onchange', e);
        }
      }
      else {
        this.setTargetWidth(targetIndex, maxWidth);
      }
    }
  }
  this.setSelectWidth(null);
}

/**
 * Returns true if an element has a specified class name
 */
Drupal.jsAS.prototype.hasClass = function (node, className) {
  if (node.className == className) {
    return true;
  }
  var reg = new RegExp('(^| )'+ className +'($| )')
  if (reg.test(node.className)) {
    return true;
  }
  return false;
}

/**
 * An ActiveSelect DataBase object
 */
Drupal.ASDB = function (uri, targets) {
  this.uri = uri;
  this.targets = targets;
  this.delay = 300;
  this.cache = {};
}

/**
 * Performs a cached and delayed search
 */
Drupal.ASDB.prototype.search = function(searchString, targets, extra) {
  this.searchString = searchString;
  if (this.cache[searchString]) {
    return this.owner.populate(this.cache[searchString]);
  }
  if (this.timer) {
    clearTimeout(this.timer);
  }
  var db = this;
  this.timer = setTimeout(function() {
    $(db.owner.input).css({
      width: db.owner.selectWidth +'px',
      backgroundPosition: (db.owner.selectWidth - 35) +'px -18px'
    });
    var targetIds = [];
    for (var target = 0; target < targets.length; target++) {
      if (targets[target].id) {
        targetIds.push($(targets[target]).id().substr(5));
      }
    }
    var targetsString = targetIds.join(',');

    // Ajax GET request for activeselect
    $.ajax({
      type: "GET",
      url: db.uri +'/'+ encodeURIComponent(targetsString) +'/'+ encodeURIComponent(searchString) +'/'+ encodeURIComponent(extra),
      success: function (data) {
        // Split into array of key->value pairs
        if (data.length > 0) {
          var targets = Drupal.parseJson(data);
          if (typeof targets['status'] == 'undefined' || targets['status'] != 0) {
            db.cache[searchString] = targets;
            db.owner.populate(targets);
          }
        }
      },
      error: function (xmlhttp) {
        asdb.owner.setSelectWidth(null);
        alert('An HTTP error '+ xmlhttp.status +' occured.\n'+ db.uri);
      }
    });
  }, this.delay);
}

// Global Killswitch
if (Drupal.jsEnabled) {
  $(document).ready(Drupal.activeselectAutoAttach);
}
