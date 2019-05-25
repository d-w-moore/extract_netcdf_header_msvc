/*
 *Backend app configuration
 *For app specific configs, prefix with beconf. for consistency 
 */


// use login preset
beconf.login.preset.host='irodshost'
beconf.login.preset.port=1247
beconf.login.preset.zone='tempZone'
beconf.login.preset.auth.type='STANDARD'
beconf.login.preset.enabled=true
beconf.negotiation.policy='CS_NEG_REFUSE' // NO_NEGOTIATION, CS_NEG_REFUSE, CS_NEG_REQ, CS_NEG_DONT_CARE
