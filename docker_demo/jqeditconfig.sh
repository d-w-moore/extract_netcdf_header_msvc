#!/bin/bash
jq --indent 4 -M '.plugin_configuration.rule_engines |= .[:1]+[
    {
        "instance_name": "irods_rule_engine_plugin-python-instance",
        "plugin_name": "irods_rule_engine_plugin-python",
        "plugin_specific_configuration": {}
    }
  ]+.[1:]' \
  < /etc/irods/server_config.json > /tmp/server_config.json && \
  cp /tmp/server_config.json /etc/irods
