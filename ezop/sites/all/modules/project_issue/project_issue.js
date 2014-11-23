/* $Id: project_issue.js,v 1.4.2.1 2008/10/26 18:36:58 thehunmonkgroup Exp $ */

Drupal.projectSwitchAutoAttach = function () {
  $('#edit-project-info-pid').change(function () {
    var pid = this.value;

    // Temporarily disable the dynamic selects while we retrieve the new info.
    // The newly loaded selects will be enabled by default.
    $('#edit-project-info-rid, #edit-project-info-component, #edit-assigned').attr('disabled', 'disabled');

    // Get existing component setting.
    var cid = $('#edit-project-info-component').val();
    cid = Drupal.encodeURIComponent(cid);

    // Get existing version label.
    var nid = $('#edit-project-info-rid').val();
    var rid = $('#edit-project-info-rid option[@value=' + nid + ']').text();
    rid = Drupal.encodeURIComponent(rid);

    // Get existing assigned uid.
    var assigned_uid = $('#edit-assigned').val();
    assigned_uid = Drupal.encodeURIComponent(assigned_uid);

    // Get the issue node's nid.
    var issue_nid = Drupal.encodeURIComponent(Drupal.settings.issueNid);

    // Pass new project ID, existing version, existing component.
    var url = Drupal.settings.projectUrl + '/' + pid + '/' + issue_nid + '/' + cid + '/' + rid + '/' + assigned_uid;

    // Ajax GET request.
    $.ajax({
      type: 'GET',
      url: url,
      success: function (data) {
        // Parse result and insert updated selects.
        var result = Drupal.parseJson(data);
        if (result.error) {
          alert(result.error);
        }
        else {
          $('#edit-project-info-rid, #edit-project-info-component').parent().remove();
          $('#edit-project-info-pid').parent().after(result.component).after(result.rid);
          $('#edit-assigned').parent().remove();
          $('#edit-priority').parent().after(result.assigned);
        }
      },
      error: function (xmlhttp) {
        alert('An HTTP error '+ xmlhttp.status +' occured.\n'+ url);
      }
    });
  });
}

// Global killswitch.
if (Drupal.jsEnabled) {
  $(document).ready(Drupal.projectSwitchAutoAttach);
}
