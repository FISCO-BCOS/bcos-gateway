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

file_must_not_exists() {
    if [ -f "$1" ]; then
        LOG_FALT "$1 file already exist, please check!"
    fi
}

file_must_exists() {
    if [ ! -f "$1" ]; then
        LOG_FALT "$1 file does not exist, please check!"
    fi
}

help() {
    cat <<EOF
Usage:
    -c <node count>                     [Optional] install node count, default 4
    -d <output dir>                     [Optional] output directory, default ./gateway-env
    -e <gateway exec>                   [Required] gateway binay exec
    -p <listenPort>                     [Optional] start listen port, default 30300
    -s <SM model>                       [Optional] SM SSL connection or not, default no
    -h Help
e.g
    bash $0 -p 30300 -s -e ./gateway-exec-mini
    bash $0 -p 30300 -s -c 10 -e ./gateway-exec-mini
EOF

    exit 0
}

listen_ip="0.0.0.0"
listen_port=30300
ssl_model=""
node_count=4
gateway_exec=""
output_dir="${dirpath}/gateway-env"

parse_params() {
    while getopts "c:d:e:l:p:sh" option; do
        case $option in
        c) node_count="$OPTARG" ;;
        d)
            output_dir="$OPTARG"
            mkdir -p "$output_dir"
            dir_must_exists "${output_dir}"
            ;;
        e) gateway_exec="$OPTARG" ;;
        l) listen_ip="$OPTARG" ;;
        p) listen_port="$OPTARG" ;;
        s) ssl_model="-s" ;;
        h) help ;;
        *) help ;;
        esac
    done
}

print_result() {
    echo "=============================================================="
    LOG_INFO "listenIP          : ${listen_ip}"
    LOG_INFO "listenPort        : ${listen_port}"
    LOG_INFO "SSL Model         : ${ssl_model}"
    LOG_INFO "node count         : ${node_count}"
    LOG_INFO "output dir         : ${output_dir}"
    LOG_INFO "All completed. Files in ${output_dir}"
}

generate_all_node_scripts() {
    local output=${1}

    cat <<EOF >"${output}/start_all.sh"
#!/bin/bash
dirpath="\$(cd "\$(dirname "\$0")" && pwd)"
cd "\${dirpath}"

dirs=(\$(ls -l \${dirpath} | awk '/^d/ {print \$NF}'))
for dir in \${dirs[*]}
do
    if [[ -f "\${dirpath}/\${dir}/config.ini" && -f "\${dirpath}/\${dir}/start.sh" ]];then
        echo "try to start \${dir}"
        bash \${dirpath}/\${dir}/start.sh 
    fi
done
wait
EOF

    cat <<EOF >"${output}/stop_all.sh"
#!/bin/bash
dirpath="\$(cd "\$(dirname "\$0")" && pwd)"
cd "\${dirpath}"

dirs=(\$(ls -l \${dirpath} | awk '/^d/ {print \$NF}'))
for dir in \${dirs[*]}
do
    if [[ -f "\${dirpath}/\${dir}/config.ini" && -f "\${dirpath}/\${dir}/stop.sh" ]];then
        echo "try to stop \${dir}"
        bash \${dirpath}/\${dir}/stop.sh 
    fi
done
wait
EOF
}

generate_node_scripts() {
    local output=${1}

    cat <<EOF >"${output}/start.sh"
#!/bin/bash
dirpath="\$(cd "\$(dirname "\$0")" && pwd)"
cd "\${dirpath}"

config=\${dirpath}/config.ini
node=\$(basename \${dirpath})
pid=\$(ps aux | grep \${config} | grep -v grep | awk '{print \$2}')
if [ -n "\${pid}" ];then
        echo "\${node} is running, pid is \${pid}"
        exit 0
fi
nohup ../gateway-exec-mini \${dirpath}/config.ini &
EOF

    cat <<EOF >"${output}/stop.sh"
#!/bin/bash
dirpath="\$(cd "\$(dirname "\$0")" && pwd)"
cd "\${dirpath}"

config=\${dirpath}/config.ini
node=\$(basename \${dirpath})
pid=\$(ps aux | grep \${config} | grep -v grep | awk '{print \$2}')
if [ -z "\${pid}" ];then
        echo "\${node} is not running"
        exit 0
fi
kill -9 "\${pid}"
echo "stop \${node} successfully"
EOF

    cat <<EOF >"${output}/check.sh"
#!/bin/bash
dirpath="\$(cd "\$(dirname "\$0")" && pwd)"
cd "\${dirpath}"

config=\${dirpath}/config.ini
node=\$(basename \${dirpath})
pid=\$(ps aux | grep \${config} | grep -v grep | awk '{print \$2}')
if [ -z "\${pid}" ];then
        echo "\${node} is not running"
else
        echo "\${node} is running, pid=\${pid}"
fi
EOF
}

generate_nodes_json() {
    local listen_ip="$1"
    local listen_port="$2"
    local count="$3"
    local p2p_host_list=""

    for ((i = 0; i < count; i++)); do
        mkdir -p "${output_dir}/node${i}"
        local delim=""
        if [[ $i == $((count - 1)) ]]; then
            delim=""
        else
            delim=","
        fi
        local port=$((listen_port + i))
        p2p_host_list="${p2p_host_list}${listen_ip}:${port}${delim}"
    done

    echo "${p2p_host_list}"
}

main() {
    parse_params "$@"
    file_must_exists "build_gateway_cert.sh"
    file_must_exists "build_gateway_config.sh"

    if [[ ! -f "$gateway_exec" ]]; then
        LOG_FALT "gateway binary exec ${gateway_exec} not exist, please input the correct path."
    fi

    mkdir -p "${output_dir}"
    cp "${gateway_exec}" "${output_dir}"
    connected_nodes=$(generate_nodes_json 127.0.0.1 "${listen_port}" "${node_count}")
    # ca cert
    bash build_gateway_cert.sh ${ssl_model} -d "${output_dir}"/ca
    # start_all.sh and stop_all.sh
    generate_all_node_scripts "${output_dir}"
    for ((i = 0; i < node_count; i++)); do
        mkdir -p "${output_dir}/node${i}"
        local port=$((listen_port + i))
        # node cert
        bash build_gateway_cert.sh ${ssl_model} -n -c "${output_dir}"/ca -d "${output_dir}/node${i}"
        # node config
        bash build_gateway_config.sh ${ssl_model} -l "${listen_ip}" -p "${port}" -H "${connected_nodes}" -d "${output_dir}/node${i}"
        # start.sh stop.sh
        generate_node_scripts "${output_dir}/node${i}"
    done

    print_result
}

main "$@"
