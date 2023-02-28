(window.webpackJsonp=window.webpackJsonp||[]).push([[17],{1191:function(e,t,r){"use strict";r.r(t),r.d(t,"keccak256",function(){return _});var n,o,t=r(91),i=r.n(t);let s=!1,a=!1;const c={debug:1,default:2,info:2,warning:3,error:4,off:5};let u=c.default,d=null;const l=function(){try{const r=[];if(["NFD","NFC","NFKD","NFKC"].forEach(t=>{try{if("test"!=="test".normalize(t))throw new Error("bad normalize")}catch(e){r.push(t)}}),r.length)throw new Error("missing "+r.join(", "));if(String.fromCharCode(233).normalize("NFD")!==String.fromCharCode(101,769))throw new Error("broken implementation")}catch(e){return e.message}return null}(),h=((r=n=n||{}).DEBUG="DEBUG",r.INFO="INFO",r.WARNING="WARNING",r.ERROR="ERROR",r.OFF="OFF",(t=o=o||{}).UNKNOWN_ERROR="UNKNOWN_ERROR",t.NOT_IMPLEMENTED="NOT_IMPLEMENTED",t.UNSUPPORTED_OPERATION="UNSUPPORTED_OPERATION",t.NETWORK_ERROR="NETWORK_ERROR",t.SERVER_ERROR="SERVER_ERROR",t.TIMEOUT="TIMEOUT",t.BUFFER_OVERRUN="BUFFER_OVERRUN",t.NUMERIC_FAULT="NUMERIC_FAULT",t.MISSING_NEW="MISSING_NEW",t.INVALID_ARGUMENT="INVALID_ARGUMENT",t.MISSING_ARGUMENT="MISSING_ARGUMENT",t.UNEXPECTED_ARGUMENT="UNEXPECTED_ARGUMENT",t.CALL_EXCEPTION="CALL_EXCEPTION",t.INSUFFICIENT_FUNDS="INSUFFICIENT_FUNDS",t.NONCE_EXPIRED="NONCE_EXPIRED",t.REPLACEMENT_UNDERPRICED="REPLACEMENT_UNDERPRICED",t.UNPREDICTABLE_GAS_LIMIT="UNPREDICTABLE_GAS_LIMIT",t.TRANSACTION_REPLACED="TRANSACTION_REPLACED","0123456789abcdef");class f{constructor(e){Object.defineProperty(this,"version",{enumerable:!0,value:e,writable:!1})}_log(e,t){var r=e.toLowerCase();null==c[r]&&this.throwArgumentError("invalid log level name","logLevel",e),u,c}debug(...e){this._log(f.levels.DEBUG,e)}info(...e){this._log(f.levels.INFO,e)}warn(...e){this._log(f.levels.WARNING,e)}makeError(e,t,o){if(a)return this.makeError("censored error",t,{});t=t||f.errors.UNKNOWN_ERROR,o=o||{};const i=[];Object.keys(o).forEach(r=>{var n=o[r];try{if(n instanceof Uint8Array){let t="";for(let e=0;e<n.length;e++)t=(t+=h[n[e]>>4])+h[15&n[e]];i.push(r+"=Uint8Array(0x"+t+")")}else i.push(r+"="+JSON.stringify(n))}catch(e){i.push(r+"="+JSON.stringify(o[r].toString()))}}),i.push("code="+t),i.push("version="+this.version);var r=e;i.length&&(e+=" ("+i.join(", ")+")");const n=new Error(e);return n.reason=r,n.code=t,Object.keys(o).forEach(function(e){n[e]=o[e]}),n}throwError(e,t,r){throw this.makeError(e,t,r)}throwArgumentError(e,t,r){return this.throwError(e,f.errors.INVALID_ARGUMENT,{argument:t,value:r})}assert(e,t,r,n){e||this.throwError(t,r,n)}assertArgument(e,t,r,n){e||this.throwArgumentError(t,r,n)}checkNormalize(e){null==e&&(e="platform missing String.prototype.normalize"),l&&this.throwError("platform missing String.prototype.normalize",f.errors.UNSUPPORTED_OPERATION,{operation:"String.prototype.normalize",form:l})}checkSafeUint53(e,t){"number"==typeof e&&(null==t&&(t="value not safe"),(e<0||9007199254740991<=e)&&this.throwError(t,f.errors.NUMERIC_FAULT,{operation:"checkSafeInteger",fault:"out-of-safe-range",value:e}),e%1)&&this.throwError(t,f.errors.NUMERIC_FAULT,{operation:"checkSafeInteger",fault:"non-integer",value:e})}checkArgumentCount(e,t,r){r=r?": "+r:"",e<t&&this.throwError("missing argument"+r,f.errors.MISSING_ARGUMENT,{count:e,expectedCount:t}),t<e&&this.throwError("too many arguments"+r,f.errors.UNEXPECTED_ARGUMENT,{count:e,expectedCount:t})}checkNew(e,t){e!==Object&&null!=e||this.throwError("missing new",f.errors.MISSING_NEW,{name:t.name})}checkAbstract(e,t){e===t?this.throwError("cannot instantiate abstract class "+JSON.stringify(t.name)+" directly; use a sub-class",f.errors.UNSUPPORTED_OPERATION,{name:e.name,operation:"new"}):e!==Object&&null!=e||this.throwError("missing new",f.errors.MISSING_NEW,{name:t.name})}static globalLogger(){return d=d||new f("logger/5.5.0")}static setCensorship(e,t){if(!e&&t&&this.globalLogger().throwError("cannot permanently disable censorship",f.errors.UNSUPPORTED_OPERATION,{operation:"setCensorship"}),s){if(!e)return;this.globalLogger().throwError("error censorship permanent",f.errors.UNSUPPORTED_OPERATION,{operation:"setCensorship"})}a=!!e,s=!!t}static setLogLevel(e){var t=c[e.toLowerCase()];null==t?f.globalLogger().warn("invalid log level - "+e):u=t}static from(e){return new f(e)}}f.errors=o,f.levels=n;const g=new f("bytes/5.5.0");function p(e){return e.toHexString}function y(t){return t.slice||(t.slice=function(){var e=Array.prototype.slice.call(arguments);return y(new Uint8Array(Array.prototype.slice.apply(t,e)))}),t}function v(e){return"number"==typeof e&&e==e&&e%1==0}function m(t){if(null==t)return!1;if(t.constructor!==Uint8Array){if("string"==typeof t)return!1;if(!v(t.length)||t.length<0)return!1;for(let e=0;e<t.length;e++){var r=t[e];if(!v(r)||r<0||256<=r)return!1}}return!0}function b(e,r){if(r=r||{},"number"==typeof e){g.checkSafeUint53(e,"invalid arrayify value");for(var t=[];e;)t.unshift(255&e),e=parseInt(String(e/256));return 0===t.length&&t.push(0),y(new Uint8Array(t))}if(w(e=p(e=r.allowMissingPrefix&&"string"==typeof e&&"0x"!==e.substring(0,2)?"0x"+e:e)?e.toHexString():e)){let t=e.substring(2);t.length%2&&("left"===r.hexPad?t="0x0"+t.substring(2):"right"===r.hexPad?t+="0":g.throwArgumentError("hex data is odd-length","value",e));var n=[];for(let e=0;e<t.length;e+=2)n.push(parseInt(t.substring(e,e+2),16));return y(new Uint8Array(n))}return m(e)?y(new Uint8Array(e)):g.throwArgumentError("invalid arrayify value","value",e)}function w(e,t){return!("string"!=typeof e||!e.match(/^0x[0-9A-Fa-f]*$/)||t&&e.length!==2+2*t)}function _(e){return"0x"+i.a.keccak_256(b(e))}},16:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(661),t),o(r(396),t),o(r(663),t),o(r(665),t),o(r(667),t),o(r(405),t),o(r(406),t)},27:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(696),t),o(r(702),t),o(r(839),t),o(r(840),t),o(r(842),t),o(r(843),t)},285:function(e,t,r){"use strict";var n=this&&this.__awaiter||function(e,s,a,c){return new(a=a||Promise)(function(r,t){function n(e){try{i(c.next(e))}catch(e){t(e)}}function o(e){try{i(c.throw(e))}catch(e){t(e)}}function i(e){var t;e.done?r(e.value):((t=e.value)instanceof a?t:new a(function(e){e(t)})).then(n,o)}i((c=c.apply(e,s||[])).next())})};Object.defineProperty(t,"__esModule",{value:!0}),t.BaseKVStore=void 0;t.BaseKVStore=class{constructor(e,t){this.provider=e,this._prefix=t}get(t){return n(this,void 0,void 0,function*(){var e=this.prefix()+"/"+t;return(yield this.provider.get())[e]})}set(e,t){e=this.prefix()+"/"+e;return this.provider.set({[e]:t})}prefix(){return this._prefix}}},396:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__setModuleDefault||(Object.create?function(e,t){Object.defineProperty(e,"default",{enumerable:!0,value:t})}:function(e,t){e.default=t}),i=this&&this.__importStar||function(e){if(e&&e.__esModule)return e;var t={};if(null!=e)for(var r in e)"default"!==r&&Object.prototype.hasOwnProperty.call(e,r)&&n(t,e,r);return o(t,e),t};Object.defineProperty(t,"__esModule",{value:!0}),t.Bech32Address=void 0;const s=i(r(104)),a=r(4),c=r(246);t.Bech32Address=class u{constructor(e){this.address=e}static shortenAddress(e,t){if(t>=e.length)return e;var r=e.indexOf("1"),n=e.slice(0,r),e=e.slice(r+1);if(t=t-n.length-3,--t<=0)return"";r=Math.floor(e.length/2);let o=e.slice(0,r),i=e.slice(r);for(;t<o.length+i.length;)(o.length+i.length)%2==1&&0<o.length?o=o.slice(0,o.length-1):i=i.slice(1);return n+"1"+o+"..."+i}static fromBech32(e,t){if(e=s.default.decode(e),t&&e.prefix!==t)throw new Error("Unmatched prefix");return new u(new Uint8Array(s.fromWords(e.words)))}static validate(e,t){if(e=s.default.decode(e).prefix,t&&t!==e)throw new Error(`Unexpected prefix (expected: ${t}, actual: ${e})`)}static defaultBech32Config(e,t="val",r="cons",n="pub",o="oper"){return{bech32PrefixAccAddr:e,bech32PrefixAccPub:e+n,bech32PrefixValAddr:e+t+o,bech32PrefixValPub:e+t+o+n,bech32PrefixConsAddr:e+t+r,bech32PrefixConsPub:e+t+r+n}}toBech32(e){var t=s.default.toWords(this.address);return s.default.encode(e,t)}toHex(e=!0){var t=a.Buffer.from(this.address).toString("hex");if(0===t.length)throw new Error("Empty address");return e?c.getAddress("0x"+t):"0x"+t}}},397:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.ChainIdHelper=void 0;class n{static parse(e){var t=e.split(n.VersionFormatRegExp).filter(Boolean);return 2!==t.length?{identifier:e,version:0}:{identifier:t[0],version:parseInt(t[1])}}static hasChainVersion(e){return n.parse(e).identifier!==e}}(t.ChainIdHelper=n).VersionFormatRegExp=/(.+)-([\d]+)/},398:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)},i=(Object.defineProperty(t,"__esModule",{value:!0}),t.defaultProtoCodec=t.ProtoCodec=void 0,r(619)),s=r(620),a=r(689),c=r(621),u=r(623),d=r(624),l=r(625);const h=r(402);o(r(402),t);class f{constructor(){this.typeUrlMap=new Map}unpackAny(e){var t,r;return this.typeUrlMap.has(e.typeUrl)?(r=(t=this.typeUrlMap.get(e.typeUrl)).decode(e.value),Object.assign(Object.assign({},e),{unpacked:r,factory:t})):new h.UnknownMessage(e.typeUrl,e.value)}registerAny(e,t){this.typeUrlMap.set(e,t)}}t.ProtoCodec=f,t.defaultProtoCodec=new f,t.defaultProtoCodec.registerAny("/cosmos.bank.v1beta1.MsgSend",i.MsgSend),t.defaultProtoCodec.registerAny("/cosmos.staking.v1beta1.MsgDelegate",s.MsgDelegate),t.defaultProtoCodec.registerAny("/cosmos.staking.v1beta1.MsgUndelegate",s.MsgUndelegate),t.defaultProtoCodec.registerAny("/cosmos.staking.v1beta1.MsgBeginRedelegate",s.MsgBeginRedelegate),t.defaultProtoCodec.registerAny("/cosmwasm.wasm.v1.MsgExecuteContract",d.MsgExecuteContract),t.defaultProtoCodec.registerAny("/cosmwasm.wasm.v1.MsgInstantiateContract",d.MsgInstantiateContract),t.defaultProtoCodec.registerAny("/cosmos.distribution.v1beta1.MsgWithdrawDelegatorReward",u.MsgWithdrawDelegatorReward),t.defaultProtoCodec.registerAny("/ibc.applications.transfer.v1.MsgTransfer",l.MsgTransfer),t.defaultProtoCodec.registerAny("/cosmos.gov.v1beta1.MsgVote",c.MsgVote),t.defaultProtoCodec.registerAny("/cosmos.authz.v1beta1.MsgGrant",a.MsgGrant),t.defaultProtoCodec.registerAny("/cosmos.authz.v1beta1.MsgRevoke",a.MsgRevoke)},402:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.UnknownMessage=void 0;const n=r(4);t.UnknownMessage=class{constructor(e,t){this._typeUrl=e,this._value=t}get typeUrl(){return this._typeUrl}get value(){return this._value}toJSON(){return{type_url:this._typeUrl,value:n.Buffer.from(this._value).toString("base64")}}}},403:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.ProtoSignDocDecoder=void 0;const n=r(100),o=r(398);t.ProtoSignDocDecoder=class i{constructor(e,t=o.defaultProtoCodec){this.signDoc=e,this.protoCodec=t}static decode(e){return new i(n.SignDoc.decode(e))}get txBody(){return this._txBody||(this._txBody=n.TxBody.decode(this.signDoc.bodyBytes)),this._txBody}get txMsgs(){var e=[];for(const t of this.txBody.messages)e.push(this.protoCodec.unpackAny(t));return e}get authInfo(){return this._authInfo||(this._authInfo=n.AuthInfo.decode(this.signDoc.authInfoBytes)),this._authInfo}get chainId(){return this.signDoc.chainId}get accountNumber(){return this.signDoc.accountNumber.toString()}toBytes(){return n.SignDoc.encode(this.signDoc).finish()}toJSON(){return{txBody:Object.assign(Object.assign({},n.TxBody.toJSON(this.txBody)),{messages:this.txMsgs.map(e=>{if(e){if(e instanceof o.UnknownMessage)return e.toJSON();if("factory"in e)return e.factory.toJSON(e.unpacked)}return e})}),authInfo:n.AuthInfo.toJSON(this.authInfo),chainId:this.chainId,accountNumber:this.accountNumber}}}},405:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(694),t)},406:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(695),t)},460:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.Hash=void 0;const n=r(125),o=r(1191),i=r(4);t.Hash=class{static sha256(e){return new Uint8Array((new n.sha256).update(e).digest())}static keccak256(e){return new i.Buffer(o.keccak256(e).replace("0x",""),"hex")}static truncHashPortion(e,t=e.length,r=0){return e.substring(0,t)+"…"+e.substring(e.length-r,e.length)}}},50:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(703),t),o(r(767),t),o(r(460),t)},661:function(e,t,r){"use strict";var o=this&&this.__awaiter||function(e,s,a,c){return new(a=a||Promise)(function(r,t){function n(e){try{i(c.next(e))}catch(e){t(e)}}function o(e){try{i(c.throw(e))}catch(e){t(e)}}function i(e){var t;e.done?r(e.value):((t=e.value)instanceof a?t:new a(function(e){e(t)})).then(n,o)}i((c=c.apply(e,s||[])).next())})};Object.defineProperty(t,"__esModule",{value:!0}),t.BaseAccount=void 0;const a=r(26);class c{constructor(e,t,r,n){this.type=e,this.address=t,this.accountNumber=r,this.sequence=n}static fetchFromRest(t,r,n=!1){return o(this,void 0,void 0,function*(){var e=yield t.get("/cosmos/auth/v1beta1/accounts/"+r,{validateStatus:function(e){return 200<=e&&e<300||404===e}});return c.fromProtoJSON(e.data,n?r:"")})}static fromProtoJSON(e,t=""){if(!e.account){if(t)return new c("",t,new a.Int(0),new a.Int(0));throw new Error("Account's address is unknown: "+JSON.stringify(e))}let r=e.account;var n=r["@type"]||"";const o=r.BaseAccount||r.baseAccount||r.base_account;var i=(r=o?o:r).account,i=(r=i?i:r).BaseVestingAccount||r.baseVestingAccount||r.base_vesting_account;if(i){const o=(r=i).BaseAccount||r.baseAccount||r.base_account;o&&(r=o)}let s=r.address;if(!s){if(!t)throw new Error("Account's address is unknown: "+JSON.stringify(e));s=t}i=r.account_number,e=r.sequence;return new c(n,s,new a.Int(i||"0"),new a.Int(e||"0"))}getType(){return this.type}getAddress(){return this.address}getAccountNumber(){return this.accountNumber}getSequence(){return this.sequence}}t.BaseAccount=c},663:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(397),t),o(r(664),t)},664:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.EthermintChainIdHelper=void 0;const n=r(397);t.EthermintChainIdHelper=class{static parse(e){var t,r=e.match("^([a-z]{1,})_{1}([1-9][0-9]*)-{1}([1-9][0-9]*)$");if(r&&4===r.length&&""!==r[1]&&!Number.isNaN(parseFloat(r[2]))&&Number.isInteger(parseFloat(r[2])))return t=n.ChainIdHelper.parse(e),Object.assign(Object.assign({},t),{ethChainId:parseFloat(r[2])});throw new Error("Invalid chainId for ethermint: "+e)}}},665:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.TendermintTxTracer=void 0;const n=r(666),o=r(4);t.TendermintTxTracer=class{constructor(e,t,r={}){this.url=e,this.wsEndpoint=t,this.options=r,this.newBlockSubscribes=[],this.txSubscribes=new Map,this.pendingQueries=new Map,this.listeners={},this.onOpen=e=>{var t,r,n,o,i;0<this.newBlockSubscribes.length&&this.sendSubscribeBlockRpc();for([r,n]of this.txSubscribes)this.sendSubscribeTxRpc(r,n.hash);for([o,i]of this.pendingQueries)this.sendQueryRpc(o,i.method,i.params);for(const s of null!=(t=this.listeners.open)?t:[])s(e)},this.onMessage=e=>{var t,r,n,o,i,s;for(const c of null!=(t=this.listeners.message)?t:[])c(e);if(e.data)try{var a=JSON.parse(e.data);if(null!=a&&a.id&&this.pendingQueries.has(a.id)&&(a.error?this.pendingQueries.get(a.id).rejector(new Error(a.error.data||a.error.message)):null!=(r=a.result)&&r.tx_result?this.pendingQueries.get(a.id).resolver(a.result.tx_result):this.pendingQueries.get(a.id).resolver(a.result),this.pendingQueries.delete(a.id)),"tendermint/event/NewBlock"===(null==(o=null==(n=null==a?void 0:a.result)?void 0:n.data)?void 0:o.type))for(const u of this.newBlockSubscribes)u.handler(a.result.data.value);"tendermint/event/Tx"===(null==(s=null==(i=null==a?void 0:a.result)?void 0:i.data)?void 0:s.type)&&null!=a&&a.id&&this.txSubscribes.has(a.id)&&(a.error?this.txSubscribes.get(a.id).rejector(new Error(a.error.data||a.error.message)):this.txSubscribes.get(a.id).resolver(a.result.data.value.TxResult.result),this.txSubscribes.delete(a.id))}catch(e){}},this.onClose=e=>{var t;for(const r of null!=(t=this.listeners.close)?t:[])r(e)},this.ws=new(this.options.wsObject||WebSocket)(this.getWsEndpoint()),this.ws.onopen=this.onOpen,this.ws.onmessage=this.onMessage,this.ws.onclose=this.onClose}getWsEndpoint(){let e=this.url;var t;return(e=e.startsWith("http")?e.replace("http","ws"):e).endsWith(this.wsEndpoint)||(t=this.wsEndpoint.startsWith("/")?this.wsEndpoint:"/"+this.wsEndpoint,e=e.endsWith("/")?e+t.slice(1):e+t),e}close(){this.ws.close()}get readyState(){switch(this.ws.readyState){case 0:return n.WsReadyState.CONNECTING;case 1:return n.WsReadyState.OPEN;case 2:return n.WsReadyState.CLOSING;case 3:return n.WsReadyState.CLOSED;default:return n.WsReadyState.NONE}}addEventListener(e,t){this.listeners[e]||(this.listeners[e]=[]),this.listeners[e].push(t)}subscribeBlock(e){this.newBlockSubscribes.push({handler:e}),1===this.newBlockSubscribes.length&&this.sendSubscribeBlockRpc()}sendSubscribeBlockRpc(){this.readyState===n.WsReadyState.OPEN&&this.ws.send(JSON.stringify({jsonrpc:"2.0",method:"subscribe",params:["tm.event='NewBlock'"],id:1}))}traceTx(t){return new Promise(e=>{this.queryTx(t).then(e).catch(()=>{}),this.subscribeTx(t).then(e)}).then(t=>new Promise(e=>{setTimeout(()=>e(t),100)}))}subscribeTx(r){const n=this.createRandomId();return new Promise((e,t)=>{this.txSubscribes.set(n,{hash:r,resolver:e,rejector:t}),this.sendSubscribeTxRpc(n,r)})}sendSubscribeTxRpc(e,t){this.readyState===n.WsReadyState.OPEN&&this.ws.send(JSON.stringify({jsonrpc:"2.0",method:"subscribe",params:[`tm.event='Tx' AND tx.hash='${o.Buffer.from(t).toString("hex").toUpperCase()}'`],id:e}))}queryTx(e){return this.query("tx",[o.Buffer.from(e).toString("base64"),!1])}query(r,n){const o=this.createRandomId();return new Promise((e,t)=>{this.pendingQueries.set(o,{method:r,params:n,resolver:e,rejector:t}),this.sendQueryRpc(o,r,n)})}sendQueryRpc(e,t,r){this.readyState===n.WsReadyState.OPEN&&this.ws.send(JSON.stringify({jsonrpc:"2.0",method:t,params:r,id:e}))}createRandomId(){return parseInt(Array.from({length:6}).map(()=>Math.floor(100*Math.random())).join(""))}}},666:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.WsReadyState=void 0,(t=t.WsReadyState||(t.WsReadyState={}))[t.CONNECTING=0]="CONNECTING",t[t.OPEN=1]="OPEN",t[t.CLOSING=2]="CLOSING",t[t.CLOSED=3]="CLOSED",t[t.NONE=4]="NONE"},667:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(398),t),o(r(403),t),o(r(693),t)},693:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.SignDocWrapper=void 0;const n=r(403),o=r(100),i=r(405);t.SignDocWrapper=class s{constructor(e){"msgs"in(this.signDoc=e)?this.mode="amino":this.mode="direct","amino"===this.mode?this.isADR36SignDoc=i.checkAndValidateADR36AminoSignDoc(this.aminoSignDoc):this.isADR36SignDoc=!1}static fromAminoSignDoc(e){return new s(e)}static fromDirectSignDoc(e){return new s(e)}static fromDirectSignDocBytes(e){return new s(o.SignDoc.decode(e))}clone(){return new s(this.signDoc)}get protoSignDoc(){if("amino"===this.mode)throw new Error("Sign doc is encoded as Amino Json");if("msgs"in this.signDoc)throw new Error("Unexpected error");return this._protoSignDoc||(this._protoSignDoc=new n.ProtoSignDocDecoder(this.signDoc)),this._protoSignDoc}get aminoSignDoc(){if("direct"===this.mode)throw new Error("Sign doc is encoded as Protobuf");if("msgs"in this.signDoc)return this.signDoc;throw new Error("Unexpected error")}get chainId(){return"direct"===this.mode?this.protoSignDoc.chainId:this.aminoSignDoc.chain_id}get memo(){return("direct"===this.mode?this.protoSignDoc.txBody:this.aminoSignDoc).memo}get fees(){var e;if("direct"!==this.mode)return this.aminoSignDoc.fee.amount;var t=[];for(const r of null!=(e=null==(e=this.protoSignDoc.authInfo.fee)?void 0:e.amount)?e:[]){if(null==r.denom||null==r.amount)throw new Error("Invalid fee");t.push({denom:r.denom,amount:r.amount})}return t}get gas(){var e;return"direct"===this.mode?null!=(e=this.protoSignDoc.authInfo.fee)&&e.gasLimit?parseInt(this.protoSignDoc.authInfo.fee.gasLimit):0:parseInt(this.aminoSignDoc.fee.gas)}}},694:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.verifyADR36Amino=t.verifyADR36AminoSignDoc=t.makeADR36AminoSignDoc=t.checkAndValidateADR36AminoSignDoc=void 0;const o=r(406),i=r(396),n=r(4),s=r(50);function a(e,t){if(!(!!(e&&e.msgs&&Array.isArray(e.msgs)&&1===e.msgs.length)&&"sign/MsgSignData"===e.msgs[0].type))return!1;if(""!==e.chain_id)throw new Error("Chain id should be empty string for ADR-36 signing");if(""!==e.memo)throw new Error("Memo should be empty string for ADR-36 signing");if("0"!==e.account_number)throw new Error('Account number should be "0" for ADR-36 signing');if("0"!==e.sequence)throw new Error('Sequence should be "0" for ADR-36 signing');if("0"!==e.fee.gas)throw new Error('Gas should be "0" for ADR-36 signing');if(0!==e.fee.amount.length)throw new Error("Fee amount should be empty array for ADR-36 signing");e=e.msgs[0];if("sign/MsgSignData"!==e.type)throw new Error("Invalid type of ADR-36 sign msg: "+e.type);if(!e.value)throw new Error("Empty value in the msg");var r=e.value.signer;if(!r)throw new Error("Empty signer in the ADR-36 msg");i.Bech32Address.validate(r,t);r=e.value.data;if(!r)throw new Error("Empty data in the ADR-36 msg");t=n.Buffer.from(r,"base64");if(t.toString("base64")!==r)throw new Error("Data is not encoded by base64");if(0===t.length)throw new Error("Empty data in the ADR-36 msg");return!0}function c(e,t){return{chain_id:"",account_number:"0",sequence:"0",fee:{gas:"0",amount:[]},msgs:[{type:"sign/MsgSignData",value:{signer:e,data:t=n.Buffer.from(t).toString("base64")}}],memo:""}}function u(e,t,r,n){if(!a(t,e))throw new Error("Invalid sign doc for ADR-36");r=new s.PubKeySecp256k1(r);if(new i.Bech32Address(r.getAddress()).toBech32(e)!==t.msgs[0].value.signer)throw new Error("Unmatched signer");e=o.serializeSignDoc(t);return r.verify(e,n)}t.checkAndValidateADR36AminoSignDoc=a,t.makeADR36AminoSignDoc=c,t.verifyADR36AminoSignDoc=u,t.verifyADR36Amino=function(e,t,r,n,o){return u(e,c(t,r),n,o)}},695:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.serializeSignDoc=t.encodeSecp256k1Signature=t.encodeSecp256k1Pubkey=void 0;const n=r(4),o=r(27);function i(e){if(33!==e.length||2!==e[0]&&3!==e[0])throw new Error("Public key must be compressed secp256k1, i.e. 33 bytes starting with 0x02 or 0x03");return{type:"tendermint/PubKeySecp256k1",value:n.Buffer.from(e).toString("base64")}}t.encodeSecp256k1Pubkey=i,t.encodeSecp256k1Signature=function(e,t){if(64!==t.length)throw new Error("Signature must be 64 bytes long. Cosmos SDK uses a 2x32 byte fixed length encoding for the secp256k1 signature integers r and s.");return{pub_key:i(e),signature:n.Buffer.from(t).toString("base64")}},t.serializeSignDoc=function(e){return n.Buffer.from(o.sortedJsonByKeyStringify(e))}},696:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(697),t),o(r(698),t),o(r(285),t),o(r(699),t),o(r(700),t),o(r(701),t)},697:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0})},698:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.ExtensionKVStore=void 0;class n extends r(285).BaseKVStore{constructor(e){if(n.KVStoreProvider||"undefined"!=typeof browser&&browser.storage&&browser.storage.local&&(n.KVStoreProvider={get:browser.storage.local.get,set:browser.storage.local.set}),!n.KVStoreProvider)throw new Error("Can't initialize kv store for browser extension");super(n.KVStoreProvider,e)}}t.ExtensionKVStore=n},699:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.MemoryKVStore=void 0;class n{constructor(){this.store={}}get(){return Promise.resolve(this.store)}set(e){return this.store=Object.assign(Object.assign({},this.store),e),Promise.resolve()}}class o extends r(285).BaseKVStore{constructor(e){super(new n,e)}}t.MemoryKVStore=o},700:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.LocalKVStore=void 0;t.LocalKVStore=class{constructor(e){this._prefix=e}get(e){e=this.prefix()+"/"+e,e=localStorage.getItem(e);return null===e?Promise.resolve(void 0):Promise.resolve(JSON.parse(e))}set(e,t){e=this.prefix()+"/"+e;return null===t?Promise.resolve(localStorage.removeItem(e)):Promise.resolve(localStorage.setItem(e,JSON.stringify(t)))}prefix(){return this._prefix}}},701:function(e,t,r){"use strict";var n=this&&this.__awaiter||function(e,s,a,c){return new(a=a||Promise)(function(r,t){function n(e){try{i(c.next(e))}catch(e){t(e)}}function o(e){try{i(c.throw(e))}catch(e){t(e)}}function i(e){var t;e.done?r(e.value):((t=e.value)instanceof a?t:new a(function(e){e(t)})).then(n,o)}i((c=c.apply(e,s||[])).next())})};Object.defineProperty(t,"__esModule",{value:!0}),t.IndexedDBKVStore=void 0;t.IndexedDBKVStore=class{constructor(e){this._prefix=e}get(o){return n(this,void 0,void 0,function*(){const n=(yield this.getDB()).transaction([this.prefix()],"readonly").objectStore(this.prefix());return new Promise((e,t)=>{const r=n.get(o);r.onerror=e=>{e.stopPropagation(),t(e.target)},r.onsuccess=()=>{r.result?e(r.result.data):e(void 0)}})})}set(i,s){return n(this,void 0,void 0,function*(){if(null===s){const n=(yield this.getDB()).transaction([this.prefix()],"readwrite").objectStore(this.prefix());return new Promise((e,t)=>{var r=n.delete(i);r.onerror=e=>{e.stopPropagation(),t(e.target)},r.onsuccess=()=>{e()}})}{const o=(yield this.getDB()).transaction([this.prefix()],"readwrite").objectStore(this.prefix());return new Promise((e,t)=>{var r=o.put({key:i,data:s});r.onerror=e=>{e.stopPropagation(),t(e.target)},r.onsuccess=()=>{e()}})}})}prefix(){return this._prefix}getDB(){return n(this,void 0,void 0,function*(){return this.cachedDB||new Promise((e,t)=>{const r=window.indexedDB.open(this.prefix());r.onerror=e=>{e.stopPropagation(),t(e.target)},r.onupgradeneeded=e=>{e.target.result.createObjectStore(this.prefix(),{keyPath:"key"})},r.onsuccess=()=>{this.cachedDB=r.result,e(r.result)}})})}}},702:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.DenomHelper=void 0;const o=r(4),i=r(50);t.DenomHelper=class{constructor(e){this._denom=e;e=this.denom.split(/(\w+):(\w+):(.+)/).filter(Boolean);if(1!==e.length&&3!==e.length)throw new Error("Invalid denom: "+this.denom);this._type=3===e.length?e[0]:"",this._contractAddress=3===e.length?e[1]:""}static ibcDenom(e,t){var r=[];for(const n of e)r.push(n.portId+"/"+n.channelId);e=r.join("/")+"/"+t;return"ibc/"+o.Buffer.from(i.Hash.sha256(o.Buffer.from(e))).toString("hex").toUpperCase()}get denom(){return this._denom}get type(){return this._type||"native"}get contractAddress(){return this._contractAddress}}},703:function(e,t,r){"use strict";var i=this&&this.__awaiter||function(e,s,a,c){return new(a=a||Promise)(function(r,t){function n(e){try{i(c.next(e))}catch(e){t(e)}}function o(e){try{i(c.throw(e))}catch(e){t(e)}}function i(e){var t;e.done?r(e.value):((t=e.value)instanceof a?t:new a(function(e){e(t)})).then(n,o)}i((c=c.apply(e,s||[])).next())})};Object.defineProperty(t,"__esModule",{value:!0}),t.Mnemonic=void 0;const n=r(105),o=r(736),s=r(292),a=r(4);class c{static generateWallet(t,r="m/44'/60'/0'/0/0",n="",o=256){return i(this,void 0,void 0,function*(){var e=yield c.generateSeed(t,o);return{privKey:c.generateWalletFromMnemonic(e,r,n),mnemonic:e}})}static validateMnemonic(e){return n.validateMnemonic(e)}static generateSeed(t,r=128){return i(this,void 0,void 0,function*(){if(r%32!=0)throw new TypeError("invalid entropy");var e=new Uint8Array(r/8),e=yield t(e);return n.entropyToMnemonic(a.Buffer.from(e).toString("hex"))})}static generateWalletFromMnemonic(e,t="m/44'/60'/0'/0/0",r=""){e=n.mnemonicToSeedSync(e,r),r=o.fromSeed(e).derivePath(t).privateKey;if(r)return r;throw new Error("null hd key")}static generateMasterSeedFromMnemonic(e,t=""){e=n.mnemonicToSeedSync(e,t),t=o.fromSeed(e);return a.Buffer.from(s.decode(t.toBase58()))}static generatePrivateKeyFromMasterSeed(e,t="m/44'/60'/0'/0/0"){e=o.fromBase58(s.encode(e)).derivePath(t).privateKey;if(e)return e;throw new Error("null hd key")}}t.Mnemonic=c},767:function(e,t,r){"use strict";var n=this&&this.__importDefault||function(e){return e&&e.__esModule?e:{default:e}};Object.defineProperty(t,"__esModule",{value:!0}),t.PubKeySecp256k1=t.PrivKeySecp256k1=void 0;const o=r(97),i=n(r(768)),s=r(4),a=r(460);t.PrivKeySecp256k1=class u{constructor(e){this.privKey=e}static generateRandomKey(){var e=new o.ec("secp256k1");return new u(s.Buffer.from(e.genKeyPair().getPrivate().toArray()))}toBytes(){return new Uint8Array(this.privKey)}getPubKey(){var e=new o.ec("secp256k1").keyFromPrivate(this.privKey);return new c(new Uint8Array(e.getPublic().encodeCompressed("array")))}sign(e){return this.signDigest32(a.Hash.sha256(e))}signDigest32(e){if(32!==e.length)throw new Error("Invalid length of digest to sign: "+e.length);return e=new o.ec("secp256k1").keyFromPrivate(this.privKey).sign(e,{canonical:!0}),new Uint8Array(e.r.toArray("be",32).concat(e.s.toArray("be",32)))}};class c{constructor(e){this.pubKey=e}toBytes(){return new Uint8Array(this.pubKey)}getAddress(){var e=i.default.SHA256(i.default.lib.WordArray.create(this.pubKey)).toString(),e=i.default.RIPEMD160(i.default.enc.Hex.parse(e)).toString();return new Uint8Array(s.Buffer.from(e,"hex"))}toKeyPair(){return new o.ec("secp256k1").keyFromPublic(s.Buffer.from(this.pubKey).toString("hex"),"hex")}verify(e,t){return this.verifyDigest32(a.Hash.sha256(e),t)}verifyDigest32(e,t){if(32!==e.length)throw new Error("Invalid length of digest to verify: "+e.length);if(64!==t.length)throw new Error("Invalid length of signature: "+t.length);var r=new o.ec("secp256k1"),n=t.slice(0,32),t=t.slice(32);return r.verify(e,{r:s.Buffer.from(n).toString("hex"),s:s.Buffer.from(t).toString("hex")},this.toKeyPair())}}t.PubKeySecp256k1=c},839:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.toGenerator=void 0,t.toGenerator=function*(e){return yield e}},840:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(841),t)},841:function(e,t,r){"use strict";var n=this&&this.__awaiter||function(e,s,a,c){return new(a=a||Promise)(function(r,t){function n(e){try{i(c.next(e))}catch(e){t(e)}}function o(e){try{i(c.throw(e))}catch(e){t(e)}}function i(e){var t;e.done?r(e.value):((t=e.value)instanceof a?t:new a(function(e){e(t)})).then(n,o)}i((c=c.apply(e,s||[])).next())})};Object.defineProperty(t,"__esModule",{value:!0}),t.Debouncer=void 0;t.Debouncer=class{static promise(t){let r;return(...e)=>n(this,void 0,void 0,function*(){if(r)return r;try{return yield r=t.apply(this,e)}finally{r=void 0}})}}},842:function(e,t,r){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.unescapeHTML=t.escapeHTML=void 0,t.escapeHTML=function(e){return e.replace(/</g,"\\u003c").replace(/>/g,"\\u003e").replace(/&/g,"\\u0026")},t.unescapeHTML=function(e){return e.replace(/\\u003c/g,"<").replace(/\\u003e/g,">").replace(/\\u0026/g,"&")}},843:function(e,t,r){"use strict";var n=this&&this.__createBinding||(Object.create?function(e,t,r,n){void 0===n&&(n=r),Object.defineProperty(e,n,{enumerable:!0,get:function(){return t[r]}})}:function(e,t,r,n){e[n=void 0===n?r:n]=t[r]}),o=this&&this.__exportStar||function(e,t){for(var r in e)"default"===r||Object.prototype.hasOwnProperty.call(t,r)||n(t,e,r)};Object.defineProperty(t,"__esModule",{value:!0}),o(r(844),t)},844:function(e,t,r){"use strict";function n(t){if("object"!=typeof t||null===t)return t;if(Array.isArray(t))return t.map(n);var e=Object.keys(t).sort();const r={};return e.forEach(e=>{r[e]=n(t[e])}),r}Object.defineProperty(t,"__esModule",{value:!0}),t.sortedJsonByKeyStringify=t.sortObjectByKey=void 0,t.sortObjectByKey=n,t.sortedJsonByKeyStringify=function(e){return JSON.stringify(n(e))}}}]);