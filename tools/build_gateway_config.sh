#!/bin/bash
dirpath="$(cd "$(dirname "$0")" && pwd)"
cd "${dirpath}"

set -e

LOG_WARN() {
    local content=${1}
    echo -e "\033[31m[WARN] ${content}\033[0m"
}

LOG_INFO() {
    local content=${1}
    echo -e "\033[32m[INFO] ${content}\033[0m"
}

LOG_FALT() {
    local content=${1}
    echo -e "\033[31m[ERROR] ${content}\033[0m"
    exit 1
}

dir_must_exists() {
    if [ ! -d "$1" ]; then
        LOG_FALT "$1 DIR does not exist, please check!"
    fi
}

check_installed() {
    type $1 >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        LOG_FALT "$1 not installed, please install it first"
    fi
}

file_must_not_exists() {
    if [ -f "$1" ]; then
        LOG_FALT "$1 file already exist, please check!"
    fi
}

help() {
    cat <<EOF
Usage:
    -l <listenIP>                       [Optional] listen ip, default "0.0.0.0"
    -p <listenPort>                     [Optional] listen port, default 30300
    -s <SM model>                       [Optional] SM SSL connection or not, default no
    -H <p2p connected host list>        [Optional] "ip1:port1,ip2:port2 ..." e.g: "127.0.0.1:30300,127.0.0.1:30301"
    -h Help
e.g
    bash $0 -l 127.0.0.1 -p 30300 -H "127.0.0.1:30303,127.0.0.1:30303"
EOF

    exit 0
}

listen_ip="0.0.0.0"
listen_port=30300
ssl_model=false
file_dir="./"
config_file_name="config.ini"
nodes_json_file_name="nodes.json"
output_dir=${dirpath}
ip_params=""
p2p_host_list=""

parse_params() {
    while getopts "d:l:p:sH:h" option; do
        case $option in
        d)
            output_dir="$OPTARG"
            mkdir -p "$output_dir"
            dir_must_exists "${output_dir}"
            file_dir="$output_dir"
            ;;
        l) listen_ip="$OPTARG" ;;
        p) listen_port="$OPTARG" ;;
        s) ssl_model=true ;;
        H) ip_params="$OPTARG" ;;
        h) help ;;
        *) help ;;
        esac
    done
}

print_result() {
    echo "=============================================================="
    LOG_INFO "ListenIP              : ${listen_ip}"
    LOG_INFO "ListenPort            : ${listen_port}"
    LOG_INFO "SSL Model             : ${ssl_model}"
    LOG_INFO "P2P Connected Hosts   : ${ip_params}"
    LOG_INFO "All completed. Files in ${output_dir}"
}

generate_config_ini() {
    local output=${1}

    cat <<EOF >"${output}"
[p2p]
    listen_ip=${listen_ip}
    listen_port=${listen_port}
    ; ssl or sm ssl
    sm_ssl=false
    nodes_path=${file_dir}
    nodes_file=${nodes_json_file_name}

[cert]
    ; directory the certificates located in
    ca_path=${file_dir}
    ; the ca certificate file
    ca_cert=ca.crt
    ; the node private key file
    node_key=node.key
    ; the node certificate file
    node_cert=node.crt
EOF
}

generate_sm_config_ini() {
    local output=${1}

    cat <<EOF >"${output}"
[p2p]
    listen_ip=${listen_ip}
    listen_port=${listen_port}
    ; ssl or sm ssl
    sm_ssl=true
    nodes_path=${file_dir}
    nodes_file=${nodes_json_file_name}

[cert]
    ; directory the certificates located in
    ca_path=./
    ; the ca certificate file
    sm_ca_cert=sm_ca.crt
    ; the node private key file
    sm_node_key=sm_node.key
    ; the node certificate file
    sm_node_cert=sm_node.crt
    ; the node private key file
    sm_ennode_key=sm_ennode.key
    ; the node certificate file
    sm_ennode_cert=sm_ennode.crt
EOF
}

generate_nodes_json() {
    local output=${1}
    local p2p_host_list=""
    LOG_INFO "ip_params: ${ip_params}"
    local ip_array=(${ip_params//,/ })
    local ip_length=${#ip_array[@]}
    for ((i = 0; i < ip_length; i++)); do
        local ip=${ip_array[i]}
        local delim=""
        if [[ $i == $((ip_length - 1)) ]]; then
            delim=""
        else
            delim=","
        fi
        p2p_host_list="${p2p_host_list}\"${ip}\"${delim}"
    done

    cat <<EOF >"${output}"
{"nodes":[${p2p_host_list}]}
EOF
}

parse_params "$@"
file_must_not_exists "${output_dir}/${config_file_name}"
file_must_not_exists "${output_dir}/${nodes_json_file_name}"

if [ "${ssl_model}" == "false" ]; then
    generate_config_ini "${output_dir}/${config_file_name}"
else
    generate_sm_config_ini "${output_dir}/${config_file_name}"
fi

generate_nodes_json "${output_dir}/${nodes_json_file_name}"
print_result
