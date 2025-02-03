---
title: "Lab记录"
date: 2025-02-02T23:53:55+08:00
categories: [技术]
---

# lab1

lab1主要是实现mapreduce结构，需要完成的是

+ `mrworker.go`: 请求coordinator发送任务，完成自己的任务，发送任务已经完成的信号
+ `mrcoordinator.go`: 初始化任务，接受worker的任务请求，分配任务
+ `rpc.go`： worker与coordinator之间通过rpc通信，因此`rpc.go`中定义的是通信过程中传递的结构，包括了`args`与`reply`。

---

首先要知道mapreduce的结构。mapreduce的提出是为了通过多台物理机器加快任务的一种分布式解决方案。因此，假设我们要处理的任务其完整的数据是`DATA`,现在将其分片得到`datas`，交给各个物理机器上。

在这些物理机器上，运行两种操作`map`与`reduce`。一般来说`map`操作运行在那些存储了输入数据(`data`)的机器上，这样就不需要传输——*经过了map操作，输入数据被转换为中间输出结果(intermediate),一种键值对的格式*。

> **根据我们的描述，可以看到map操作其实是具有并行性的，因为我们把大的`DATA`分片成了`datas`**交给了多个物理机器同时处理。 

**map得到的结果并不是最终任务的需求**，最终的结果由`reduce`节点得到。在设计上,map得到的中间kv会经历一个分桶操作(buckets)。

> - **分桶机制**：Map 任务将中间键值对分配到 `nReduce` 个 buckets 中，每个 bucket 对应一个 Reduce 任务。这样，Reduce 任务可以**并行处理不同的 buckets**，而不需要等待其他 Reduce 任务完成。

---

再说明一下mapreduce结构的效率优势

**1）数据局部性（Data Locality）**

- **Map 任务**：Map 任务通常在存储输入数据的节点上运行，这样可以减少数据传输的开销（数据局部性）。
- **Reduce 任务**：Reduce 任务从多个 Map 任务的输出中读取数据，但由于数据已经按 bucket 分组，Reduce 任务只需要读取与自己相关的 bucket 文件，减少了不必要的数据传输。

**（2）负载均衡（Load Balancing）**

- **均匀分布**：通过哈希函数（如 `ihash(key) % nReduce`）将键值对均匀分配到不同的 buckets 中，可以确保每个 Reduce 任务的工作量大致相同，避免某些 Reduce 任务过载而其他任务空闲的情况。
- **动态调整**：如果某些 Reduce 任务处理速度较慢，系统可以动态调整任务分配，进一步提高负载均衡。

**（3）减少数据移动**

- **中间文件的本地化**：Map 任务生成的中间文件通常存储在本地磁盘上，Reduce 任务可以直接从这些文件中读取数据，而不需要通过网络传输大量数据。
- **按需读取**：Reduce 任务只需要读取与自己相关的 bucket 文件，而不是所有 Map 任务的输出文件，从而减少了数据读取的开销。

4) **键值对的聚合**

- **相同键的聚合**：通过分桶机制，所有具有相同键的键值对会被分配到同一个 bucket 中，从而确保这些键值对由同一个 Reduce 任务处理。这是 Reduce 阶段的核心需求。
- **减少重复计算**：如果没有分桶机制，Reduce 任务可能需要从所有 Map 任务的输出中查找相同键的键值对，这会增加计算和通信的开销。

**（5）任务的独立性**

- **Map 任务的独立性**：每个 Map 任务独立处理自己的输入分片，生成中间文件，不需要与其他 Map 任务通信。
- **Reduce 任务的独立性**：每个 Reduce 任务独立处理自己的 bucket，不需要与其他 Reduce 任务通信。

**（6）容错性**

- **任务重试**：如果某个 Map 任务或 Reduce 任务失败，系统可以重新分配该任务，而不需要重新运行整个作业。分桶机制使得任务的重试更加高效，因为只需要重新处理失败任务的 bucket。

---

有关实现，我采用的是`mutex`的方式，网上也有`lock-free`

首先就是要定义rpc通信的基本结构

```go
type WorkerArgs struct {
	MapTaskNumber    int // finished maptask number
	ReduceTaskNumber int // finished reducetask number
}

type WorkerReply struct {
	Tasktype int // 0: map task, 1: reduce task, 2: waiting, 3: job finished
	NMap     int // number of map task
	NReduce  int // number of reduce task

	MapTaskNumber int    // map task only
	Filename      string // maptask only

	ReduceTaskNumber int // reducetask only
}
```

然后依照，我们的逻辑，我们需要一个`Coordinator`，它需要有锁等信息

```go
type Coordinator struct {
	// Your definitions here.
	nReduce        int // number of reduce task
  nMap           int // number of map task（由输入的文件数决定）
	files          []string
	mapfinished    int        // number of finished map task
	maptasklog     []int      // log for map task, 0: not allocated, 1: waiting, 2:finished
	reducefinished int        // number of finished map task
	reducetasklog  []int      // log for reduce task
	mu             sync.Mutex // lock
}

```

接下来定义分配任务的逻辑：当worker的请求任务来到时，只要还有任务可以分配(首先判断`mapfinished < nMap`，只有当map全部都被分配了或者完成了，才把考虑reduce任务)

```go
unc (m *Master) AllocateTask(args *WorkerArgs, reply *WorkerReply) error {
	m.mu.Lock()
	if m.mapfinished < m.nMap {
		// allocate new map task
		allocate := -1 
		for i := 0; i < m.nMap; i++ {
			if m.maptasklog[i] == 0 {
				allocate = i
				break
			}
		}
		if allocate == -1 { //allocate=-1表明此时workers正在处理map任务，但是还未全部处理完，本次请求任务的worker进入等待⌛️任务的阶段。
			// waiting for unfinished map jobs
			reply.Tasktype = 2
			m.mu.Unlock()
		} else {
			// allocate map jobs
			reply.NReduce = m.nReduce
			reply.Tasktype = 0
			reply.MapTaskNumber = allocate
			reply.Filename = m.files[allocate]
			m.maptasklog[allocate] = 1 // waiting
			m.mu.Unlock()              // avoid deadlock
			go func() { //checktimeout的协程，如果超过10秒没有完成任务，就转交
				time.Sleep(time.Duration(10) * time.Second) // wait 10 seconds
				m.mu.Lock()
				if m.maptasklog[allocate] == 1 {
					// still waiting, assume the map worker is died
					m.maptasklog[allocate] = 0
				}
				m.mu.Unlock()
			}()
		}
	} else if m.mapfinished == m.nMap && m.reducefinished < m.nReduce {
		// allocate new reduce task
		allocate := -1
		for i := 0; i < m.nReduce; i++ {
			if m.reducetasklog[i] == 0 {
				allocate = i
				break
			}
		}
		if allocate == -1 {
			// waiting for unfinished reduce jobs
			reply.Tasktype = 2
			m.mu.Unlock()
		} else {
			// allocate reduce jobs
			reply.NMap = m.nMap
			reply.Tasktype = 1
			reply.ReduceTaskNumber = allocate
			m.reducetasklog[allocate] = 1 // waiting
			m.mu.Unlock()
			go func() {
				time.Sleep(time.Duration(10) * time.Second) // wait 10 seconds
				m.mu.Lock()
				if m.reducetasklog[allocate] == 1 {
					// still waiting, assume the reduce worker is died
					m.reducetasklog[allocate] = 0
				}
				m.mu.Unlock()
			}()
		}
	} else {
		reply.Tasktype = 3
		m.mu.Unlock()
	}
	return nil
}
```

好了，现在我们有了一个如何分配任务的方法，下面该来看看worker如何请求任务: worker都处于一个无限循环中，只要空闲就不断向coordinator请求任务，根据返回的任务类型来决定不同的处理。

```go
func Worker(mapf func(string, string) []KeyValue,
	reducef func(string, []string) string) {

	// Your worker implementation here.

	// uncomment to send the Example RPC to the master.
	// CallExample()

	// periodically ask master for task
	for {
		args := WorkerArgs{}
		reply := WorkerReply{}
		ok := call("Coordinator.AllocateTask", &args, &reply)
		if !ok || reply.Tasktype == 3 {
			// the master may died, which means the job is finished
			break
		}
		if reply.Tasktype == 0 {
			// map task
			intermediate := []KeyValue{}

			// open && read the file
			file, err := os.Open(reply.Filename)
			if err != nil {
				log.Fatalf("cannot open %v", reply.Filename)
			}
			content, err := ioutil.ReadAll(file)
			if err != nil {
				log.Fatalf("cannot read %v", reply.Filename)
			}
			file.Close()

			// call mapf
			kva := mapf(reply.Filename, string(content))
			intermediate = append(intermediate, kva...)

			// hash into buckets
			buckets := make([][]KeyValue, reply.NReduce) //将map任务的中间kv进行分桶操作
			for i := range buckets {
				buckets[i] = []KeyValue{}
			}
			for _, kva := range intermediate {
				buckets[ihash(kva.Key)%reply.NReduce] = append(buckets[ihash(kva.Key)%reply.NReduce], kva) //将kva.Key先进行哈希再求余，决定了分配到哪个bucket中去。
			}

			// write into intermediate files
			for i := range buckets {
				oname := "mr-" + strconv.Itoa(reply.MapTaskNumber) + "-" + strconv.Itoa(i)
				ofile, _ := ioutil.TempFile("", oname+"*")
				enc := json.NewEncoder(ofile)
				for _, kva := range buckets[i] {
					err := enc.Encode(&kva)
					if err != nil {
						log.Fatalf("cannot write into %v", oname)
					}
				}
				os.Rename(ofile.Name(), oname)
				ofile.Close()
			}
			// call master to send the finish message
			finishedArgs := WorkerArgs{reply.MapTaskNumber, -1}
			finishedReply := ExampleReply{}
			call("Coordinator.ReceiveFinishedMap", &finishedArgs, &finishedReply) //发送一个任务完成的消息给coordinator
		} else if reply.Tasktype == 1 {
			// reduce task
			// collect key-value from mr-X-Y
			intermediate := []KeyValue{}
			for i := 0; i < reply.NMap; i++ {
				iname := "mr-" + strconv.Itoa(i) + "-" + strconv.Itoa(reply.ReduceTaskNumber)
				// open && read the file
				file, err := os.Open(iname)
				if err != nil {
					log.Fatalf("cannot open %v", file)
				}
				dec := json.NewDecoder(file)
				for {
					var kv KeyValue
					if err := dec.Decode(&kv); err != nil {
						break
					}
					intermediate = append(intermediate, kv)
				}
				file.Close()
			}
			// sort by key
			sort.Sort(ByKey(intermediate))

			// output file
			oname := "mr-out-" + strconv.Itoa(reply.ReduceTaskNumber)
			ofile, _ := ioutil.TempFile("", oname+"*")

			//
			// call Reduce on each distinct key in intermediate[],
			// and print the result to mr-out-0.
			//
			i := 0
			for i < len(intermediate) {
				j := i + 1
				for j < len(intermediate) && intermediate[j].Key == intermediate[i].Key {
					j++
				}
				values := []string{}
				for k := i; k < j; k++ {
					values = append(values, intermediate[k].Value)
				}
				output := reducef(intermediate[i].Key, values)

				// this is the correct format for each line of Reduce output.
				fmt.Fprintf(ofile, "%v %v\n", intermediate[i].Key, output)

				i = j
			}
			os.Rename(ofile.Name(), oname)
			ofile.Close()

			for i := 0; i < reply.NMap; i++ {
				iname := "mr-" + strconv.Itoa(i) + "-" + strconv.Itoa(reply.ReduceTaskNumber)
				err := os.Remove(iname)
				if err != nil {
					log.Fatalf("cannot open delete" + iname)
				}
			}

			// send the finish message to master
			finishedArgs := WorkerArgs{-1, reply.ReduceTaskNumber}
			finishedReply := ExampleReply{}
			call("Master.ReceiveFinishedReduce", &finishedArgs, &finishedReply)
		}
		time.Sleep(time.Second)
	}
	return
}
```

