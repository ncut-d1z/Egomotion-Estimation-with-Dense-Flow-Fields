#!/usr/bin/env bash
# 文档级静态追踪核查；只读，不构建、不执行 CTest/算法，也不安装依赖。
# 用法：bash scripts/check_docs_traceability.sh
set -euo pipefail
cd "$(dirname "$0")/.."
export LC_ALL=C

source_dir="origin/egomotion_estimation_dense_flow_fields_matlab_scripts"
if [[ ! -d "$source_dir" ]]; then
  printf 'FAIL: 找不到原始 MATLAB 目录：%s\n' "$source_dir" >&2
  exit 2
fi
for doc in docs/REQUIREMENTS.md docs/TEST_PLAN.md docs/PLAN.md \
           docs/OBJECT_MODEL.md docs/ARCHITECTURE.md; do
  if [[ ! -f "$doc" ]]; then
    printf 'FAIL: 缺少必需的文档：%s\n' "$doc" >&2
    exit 2
  fi
done

printf '文档静态审查：%s\n' "$(git rev-parse --verify HEAD 2>/dev/null || printf 'unknown SHA')"
# 预期数量是当前已版本化的规格基线；修改编号时须连同 SRS/计划一起评审。
awk -v EXPECT_REQ=66 -v EXPECT_P=98 -v EXPECT_M=45 '
function strip(s) {
  gsub(/^[[:space:]]+|[[:space:]]+$/, "", s)
  return s
}
function issue(message) {
  failures++
  print "FAIL: " message
}
# 将 REQ 等标识符从一列中提取到集合；返回匹配项数量。
function collect(raw, prefix, values,   rest, pattern, token, n) {
  rest=raw
  pattern=prefix "[A-Z0-9-]+"
  n=0
  while (match(rest, pattern)) {
    token=substr(rest, RSTART, RLENGTH)
    values[token]=1
    rest=substr(rest, RSTART + RLENGTH)
    n++
  }
  return n
}
function checkSourceRow(raw, kind,  cells, name) {
  split(raw, cells, "|")
  name=strip(cells[2])
  # 使用八进制 \140 表示 Markdown 行内代码符号，避免正则转义歧义。
  if (name ~ /^\140[^\140]+[.]m\140$/) {
    name=substr(name, 2, length(name)-2)
    if (kind=="requirements") {
      if (name in srsSource) issue("SRS .m 重复: " name)
      srsSource[name]=1
      countSrsSource++
    } else {
      if (name in objSource) issue("对象模型 .m 重复: " name)
      objSource[name]=1
      countObjSource++
    }
  }
}
function collectTaskIds(raw, values,   rest, id) {
  rest=raw
  while (match(rest, /P[0-9]+[.][0-9]+/)) {
    id=substr(rest, RSTART, RLENGTH)
    values[id]=1
    rest=substr(rest, RSTART + RLENGTH)
  }
}
function checkTaskRow(raw,   fields, id, deps, refs, tcs, acs, r, key, nref) {
  split(raw, fields, "|")
  id=strip(fields[2])
  if (id in tasks) issue("PLAN 任务 ID 重复: " id)
  tasks[id]=1
  countTasks++

  split("", deps)
  split("", refs)
  split("", tcs)
  split("", acs)
  collectTaskIds(fields[4], deps)
  nref=collect(fields[5], "REQ-", refs)
  collect(fields[6], "TC-", tcs)
  collect(fields[6], "AC-", acs)
  if (nref==0) issue(id " 没有 REQ 追踪")
  for (key in deps) taskDep[id SUBSEP key]=1
  for (r in refs) {
    taskReq[id SUBSEP r]=1
    reqUsed[r]=1
    if (!(("TC-" substr(r, 5)) in tcs)) issue(id " 缺配套 TC: " r)
    if (!(("AC-" substr(r, 5)) in acs)) issue(id " 缺配套 AC: " r)
  }
  for (key in tcs) if (!(("REQ-" substr(key, 4)) in refs))
    issue(id " 包含没有对应 REQ 的 " key)
  for (key in acs) if (!(("REQ-" substr(key, 4)) in refs))
    issue(id " 包含没有对应 REQ 的 " key)
}
function walkTask(id,   key, parts) {
  if (taskVisiting[id]) { issue("PLAN 依赖有环，涉及: " id); return }
  if (taskVisited[id]) return
  taskVisiting[id]=1
  for (key in taskDep) {
    split(key, parts, SUBSEP)
    if (parts[1]==id && (parts[2] in tasks)) walkTask(parts[2])
  }
  taskVisiting[id]=0
  taskVisited[id]=1
}
function walkTarget(id,   key, parts) {
  if (targetVisiting[id]) { issue("HLD target DAG 有环，涉及: " id); return }
  if (targetVisited[id]) return
  targetVisiting[id]=1
  for (key in targetDep) {
    split(key, parts, SUBSEP)
    if (parts[1]==id) walkTarget(parts[2])
  }
  targetVisiting[id]=0
  targetVisited[id]=1
}
FILENAME=="docs/REQUIREMENTS.md" {
  if ($0 ~ /^\|[[:space:]]*REQ-[A-Z0-9-]+[[:space:]]*\|/) {
    split($0, f, "|")
    id=strip(f[2])
    if (id in requirements) issue("REQ 重复: " id)
    requirements[id]=1
    countReq++
    want="AC-" substr(id, 5)
    split("", found)
    nac=collect(f[6], "AC-", found)
    if (!(want in found) || nac!=1) issue("SRS AC 不匹配: " id)
  }
  checkSourceRow($0, "requirements")
  next
}
FILENAME=="docs/TEST_PLAN.md" {
  if ($0 ~ /^\|[[:space:]]*TC-[A-Z0-9-]+[[:space:]]*\|/) {
    split($0, f, "|")
    id=strip(f[2])
    if (id in tests) issue("TC 重复: " id)
    tests[id]=1
    countTests++
    wantReq="REQ-" substr(id, 4)
    wantAc="AC-" substr(id, 4)
    if (strip(f[3])!=wantReq || strip(f[6])!=wantAc)
      issue("TEST_PLAN TC→REQ/AC 错配: " id)
  }
  next
}
FILENAME=="docs/PLAN.md" {
  if ($0 ~ /^\|[[:space:]]*P[0-9]+[.][0-9]+[[:space:]]*\|/) checkTaskRow($0)
  next
}
FILENAME=="docs/OBJECT_MODEL.md" {
  checkSourceRow($0, "object")
  next
}
FILENAME=="docs/ARCHITECTURE.md" {
  if ($0 ~ /^### 4[.]1 /) inTargetDAG=1
  if ($0 ~ /^### 4[.]2 /) inTargetDAG=0
  if (inTargetDAG && $0 ~ / --> /) {
    n=split($0, parts, / --> /)
    if (n!=2) { issue("HLD DAG 边解析失败: " $0); next }
    source=strip(parts[1])
    dest=strip(parts[2])
    sub(/\[.*/, "", source)
    sub(/\[.*/, "", dest)
    if (source !~ /^[A-Za-z][A-Za-z0-9_]*$/ ||
        dest !~ /^[A-Za-z][A-Za-z0-9_]*$/) {
      issue("HLD DAG 节点格式无法解析: " $0)
    } else {
      targetDep[source SUBSEP dest]=1
      targetNodes[source]=1
      targetNodes[dest]=1
      targetEdges++
    }
  }
  next
}
# 最后一个输入是 find 输出的 .m 路径（去掉 origin 根目录前缀）。
{
  name=strip($0)
  if (name ~ /[.]m$/) {
    if (name in originalSources) issue("原始 .m 文件重复: " name)
    originalSources[name]=1
    countOriginal++
  }
}
END {
  if (countReq!=EXPECT_REQ) issue("REQ 计数不符: " countReq " != " EXPECT_REQ)
  if (countTests!=EXPECT_REQ) issue("TC 计数不符: " countTests " != " EXPECT_REQ)
  if (countTasks!=EXPECT_P) issue("PLAN 计数不符: " countTasks " != " EXPECT_P)
  if (countOriginal!=EXPECT_M) issue("原始 .m 计数不符: " countOriginal " != " EXPECT_M)
  if (countSrsSource!=EXPECT_M || countObjSource!=EXPECT_M)
    issue(".m 映射数不符: SRS=" countSrsSource " OBJECT=" countObjSource)
  for (r in requirements) {
    tc="TC-" substr(r, 5)
    if (!(tc in tests)) issue("REQ 缺对应 TC: " r)
    if (!(r in reqUsed)) issue("REQ 未被任何 P 任务引用: " r)
  }
  for (tc in tests) {
    r="REQ-" substr(tc, 4)
    if (!(r in requirements)) issue("多余的 TC: " tc)
  }
  for (pair in taskReq) {
    split(pair, item, SUBSEP)
    if (!(item[2] in requirements))
      issue("P 任务引用不存在的 REQ: " item[1] " -> " item[2])
  }
  for (pair in taskDep) {
    split(pair, item, SUBSEP)
    if (!(item[2] in tasks)) issue("P 任务引用不存在的前置项: " item[1] " -> " item[2])
  }
  for (p in tasks) walkTask(p)
  for (path in originalSources) {
    if (!(path in srsSource)) issue("SRS 缺 .m 映射: " path)
    if (!(path in objSource)) issue("OBJECT_MODEL 缺 .m 映射: " path)
  }
  for (path in srsSource) if (!(path in originalSources))
    issue("SRS 映射了不存在的 .m: " path)
  for (path in objSource) if (!(path in originalSources))
    issue("OBJECT_MODEL 映射了不存在的 .m: " path)
  if (targetEdges==0) issue("没有解析到 HLD §4.1 target DAG")
  for (p in targetNodes) walkTarget(p)
  printf "统计: REQ=%d TC=%d P=%d 原始.m=%d SRS映射=%d 对象映射=%d HLD target边=%d\n", \
    countReq, countTests, countTasks, countOriginal, countSrsSource, countObjSource, targetEdges
  if (failures) {
    printf "文档静态审查失败: %d 个不一致（未运行 CMake/CTest）。\n", failures
    exit 1
  }
  print "PASS: 文档级 ID/双向追踪/依赖环/源 .m 映射集合检查一致（非代码/算法验收）。"
}
' docs/REQUIREMENTS.md docs/TEST_PLAN.md docs/PLAN.md \
  docs/OBJECT_MODEL.md docs/ARCHITECTURE.md \
  <(find "$source_dir" -type f -name '*.m' -print \
    | sed "s#^$source_dir/##" | sort)
